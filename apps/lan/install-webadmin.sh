#!/usr/bin/env bash
#
# Installs the web admin panel (apps/webadmin, from marrenia/azerothcore-webadmin) on the Debian server.
# Run it from the cloned repository as debwow (it asks for sudo), after install-debian.sh and after the
# worldserver has started once (that creates the mod-player-statistics tables the panel needs):
#
#   bash apps/lan/install-webadmin.sh
#
# The panel listens on 127.0.0.1:8090 only. It is reached from outside through the Cloudflare tunnel
# (apps/lan/install-tunnel.sh) at https://ermewow.terpstack.dev, which terminates TLS, so the panel's own
# nginx proxy is not installed. Running it again updates the panel and keeps every password.

set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
INSTALL_DIR="${INSTALL_DIR:-$HOME/azeroth-server}"
WORLD_CONF="$INSTALL_DIR/etc/worldserver.conf"
PANEL_PORT="${PANEL_PORT:-8090}"
SOAP_ACCOUNT="WEBADMINSOAP"
SOAP_ENV="/etc/acore/soap.env"

step() { echo; echo "==> $*"; }

if [ "$EUID" -eq 0 ]; then
    echo "Run this as your normal user (debwow), not as root. It uses sudo where needed."
    exit 1
fi

[ -f "$WORLD_CONF" ] || { echo "$WORLD_CONF not found. Run apps/lan/install-debian.sh first."; exit 1; }

if ! sudo mysql -N -e "SELECT 1 FROM acore_characters.mod_player_stats_events LIMIT 1" >/dev/null 2>&1; then
    echo "The mod-player-statistics tables don't exist yet."
    echo "Run apps/lan/install-debian.sh again (it builds the module in), start the servers once with"
    echo "apps/lan/start.sh, wait until the worldserver is up, then run this script again."
    exit 1
fi

step "Enabling SOAP on 127.0.0.1:7878 in worldserver.conf (the panel sends GM commands through it)"
restart_needed=0
if ! grep -qE '^SOAP\.Enabled *= *1' "$WORLD_CONF"; then
    sed -i 's|^SOAP\.Enabled *=.*|SOAP.Enabled = 1|' "$WORLD_CONF"
    restart_needed=1
fi
sed -i 's|^SOAP\.IP *=.*|SOAP.IP = "127.0.0.1"|; s|^SOAP\.Port *=.*|SOAP.Port = 7878|' "$WORLD_CONF"

# Keep the SOAP account's password if a working one was saved before
soap_pass=""
if sudo test -f "$SOAP_ENV"; then
    soap_pass="$(sudo grep -E '^SOAP_PASS=' "$SOAP_ENV" | cut -d= -f2-)"
    [ "$soap_pass" = "CHANGEME" ] && soap_pass=""
fi
if [ -z "$soap_pass" ]; then
    step "Creating the GM account $SOAP_ACCOUNT the panel uses for SOAP"
    # Game passwords are case-insensitive and at most 16 characters
    soap_pass="$(openssl rand -hex 8 | tr 'a-f' 'A-F')"
    python3 - "$SOAP_ACCOUNT" "$soap_pass" <<'PY' | sudo mysql
import hashlib, secrets, sys
user, password = sys.argv[1].upper(), sys.argv[2].upper()
N = int("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7", 16)
salt = secrets.token_bytes(32)
h1 = hashlib.sha1(f"{user}:{password}".encode()).digest()
x = int.from_bytes(hashlib.sha1(salt + h1).digest(), "little")
verifier = pow(7, x, N).to_bytes(32, "little")
print(f"""INSERT INTO acore_auth.account (username, salt, verifier, expansion)
VALUES ('{user}', 0x{salt.hex()}, 0x{verifier.hex()}, 2)
ON DUPLICATE KEY UPDATE salt = VALUES(salt), verifier = VALUES(verifier);
REPLACE INTO acore_auth.account_access (id, gmlevel, RealmID, comment)
SELECT id, 3, -1, 'web admin SOAP' FROM acore_auth.account WHERE username = '{user}';""")
PY
    # The panel's installer keeps an existing soap.env, so drop the old one to store the new password
    sudo rm -f "$SOAP_ENV"
fi

step "Installing the panel (systemd service acore-webadmin, app on 127.0.0.1:$PANEL_PORT)"
sudo env TLS_MODE=none BIND_ADDR=127.0.0.1 PORT="$PANEL_PORT" \
    SOAP_USER="$SOAP_ACCOUNT" SOAP_PASS="$soap_pass" \
    BOT_ACCOUNT_PREFIX=rndbot \
    bash "$REPO_DIR/apps/webadmin/deploy/install.sh"

step "Telling the panel it runs behind Cloudflare (HTTPS cookies, real client IPs for the login lockout)"
sudo mkdir -p /etc/systemd/system/acore-webadmin.service.d
printf '[Service]\nEnvironment=ACORE_WEBADMIN_BEHIND_TLS_PROXY=1\n' \
    | sudo tee /etc/systemd/system/acore-webadmin.service.d/cloudflare-tunnel.conf >/dev/null
sudo systemctl daemon-reload
sudo systemctl restart acore-webadmin

step "Done"
echo "The panel login is printed above (user admin; the password is kept in /etc/acore/webadmin.env)."
echo "Any game account logs in too; its GM level decides what it can do."
echo "Make it reachable at https://ermewow.terpstack.dev with:  bash $REPO_DIR/apps/lan/install-tunnel.sh"
if [ "$restart_needed" = 1 ]; then
    echo
    echo "SOAP was just switched on: restart the servers so GM commands from the panel work:"
    echo "  bash $REPO_DIR/apps/lan/start.sh stop && bash $REPO_DIR/apps/lan/start.sh"
fi
