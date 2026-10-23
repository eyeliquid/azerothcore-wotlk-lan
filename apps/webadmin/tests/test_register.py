"""Self-registration: off by default, validation, reserved names, and the insert it makes."""
import pytest

import core


@pytest.fixture
def reg(app, monkeypatch):
    import app as app_module
    monkeypatch.setattr(app_module, "REGISTRATION_ENABLED", True)
    monkeypatch.setitem(app.jinja_env.globals, "REGISTRATION_ENABLED", True)
    app_module._registrations.clear()
    inserts = []
    lookups = {"existing": None}

    def fake_query(sql, args=(), one=False):
        if "FROM acore_auth.account WHERE username" in sql:
            return lookups["existing"]
        if "realmlist" in sql:
            return {"address": "192.168.2.35"}
        raise AssertionError(f"unexpected query: {sql}")

    def fake_execute(sql, args=()):
        inserts.append((sql, args))
        return 1

    monkeypatch.setattr(app_module, "query", fake_query)
    monkeypatch.setattr(app_module, "execute", fake_execute)
    return app_module, inserts, lookups


def _post(client, **form):
    client.get("/register")
    with client.session_transaction() as s:
        token = s["csrf"]
    return client.post("/register", data={"_csrf": token, **form})


def test_registration_is_off_by_default(client):
    assert client.get("/register").status_code == 404


def test_register_creates_player_account(client, reg):
    _, inserts, _ = reg
    resp = _post(client, username="Thrall", password="lokt4r", confirm="lokt4r")
    assert resp.status_code == 200
    assert b"set realmlist 192.168.2.35" in resp.data
    assert len(inserts) == 1
    sql, args = inserts[0]
    assert "INSERT INTO acore_auth.account" in sql
    uname, salt, verifier = args[0], args[1], args[2]
    assert uname == "THRALL"
    assert core.srp6_make("THRALL", "lokt4r", salt=salt)[1] == verifier
    assert "account_access" not in sql


@pytest.mark.parametrize("form, message", [
    ({"username": "ab", "password": "secret1", "confirm": "secret1"}, b"at least 3"),
    ({"username": "player", "password": "short", "confirm": "short"}, b"at least 6"),
    ({"username": "player", "password": "secret1", "confirm": "secret2"}, b"not the same"),
    ({"username": "bad name", "password": "secret1", "confirm": "secret1"}, b"letters and numbers"),
    ({"username": "rndbot999", "password": "secret1", "confirm": "secret1"}, b"reserved"),
    ({"username": "player", "password": "x" * 17, "confirm": "x" * 17}, b"at most 16"),
])
def test_register_rejects_bad_input(client, reg, form, message):
    _, inserts, _ = reg
    resp = _post(client, **form)
    assert resp.status_code == 400
    assert message in resp.data
    assert not inserts


def test_register_rejects_existing_account(client, reg):
    _, inserts, lookups = reg
    lookups["existing"] = {"id": 1}
    resp = _post(client, username="Jaina", password="secret1", confirm="secret1")
    assert resp.status_code == 400
    assert b"already exists" in resp.data
    assert not inserts


def test_register_limits_accounts_per_address(client, reg):
    app_module, inserts, _ = reg
    for i in range(app_module.MAX_REGISTRATIONS_PER_IP):
        assert _post(client, username=f"alt{i}", password="secret1", confirm="secret1").status_code == 200
    assert _post(client, username="onemore", password="secret1", confirm="secret1").status_code == 429
    assert len(inserts) == app_module.MAX_REGISTRATIONS_PER_IP


def test_login_page_links_to_register_when_enabled(client, reg):
    assert b'href="/register"' in client.get("/login").data
