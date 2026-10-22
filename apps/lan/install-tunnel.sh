#!/usr/bin/env bash
#
# Puts the web admin panel online at https://ermewow.terpstack.dev through a Cloudflare tunnel.
# No ports are opened on the router or the server: cloudflared connects out to Cloudflare.
# Run it from the cloned repository as debwow (it asks for sudo), after install-webadmin.sh:
#
#   bash apps/lan/install-tunnel.sh
#
# The first run prints a Cloudflare login link: open it on any device, log in and pick terpstack.dev.
# The script then creates the tunnel "ermewow", points ermewow.terpstack.dev at it and installs
# cloudflared as a systemd service. Running it again reuses the tunnel and rewrites the config.
#
# A tunnel made in the Cloudflare dashboard (Zero Trust > Networks > Tunnels) works too: give its token
# and set the public hostname ermewow.terpstack.dev -> http://127.0.0.1:8090 in the dashboard:
#
#   CF_TUNNEL_TOKEN=eyJ... bash apps/lan/install-tunnel.sh
#
# Only the web panel goes through the tunnel. The game itself (ports 3724 and 8085) stays on the LAN.

set -euo pipefail

TUNNEL_NAME="${TUNNEL_NAME:-ermewow}"
TUNNEL_HOSTNAME="${TUNNEL_HOSTNAME:-ermewow.terpstack.dev}"
ORIGIN="${ORIGIN:-http://127.0.0.1:8090}"
CF_TUNNEL_TOKEN="${CF_TUNNEL_TOKEN:-}"
CF_DIR="/etc/cloudflared"

step() { echo; echo "==> $*"; }

if [ "$EUID" -eq 0 ]; then
    echo "Run this as your normal user (debwow), not as root. It uses sudo where needed."
    exit 1
fi

if ! command -v cloudflared >/dev/null 2>&1; then
    step "Installing cloudflared (Cloudflare's apt repository)"
    sudo mkdir -p --mode=0755 /usr/share/keyrings
    curl -fsSL https://pkg.cloudflare.com/cloudflare-main.gpg \
        | sudo tee /usr/share/keyrings/cloudflare-main.gpg >/dev/null
    echo "deb [signed-by=/usr/share/keyrings/cloudflare-main.gpg] https://pkg.cloudflare.com/cloudflared any main" \
        | sudo tee /etc/apt/sources.list.d/cloudflared.list >/dev/null
    sudo apt-get update -y
    sudo apt-get install -y cloudflared
fi

service_installed() { systemctl list-unit-files cloudflared.service >/dev/null 2>&1 \
    && systemctl list-unit-files cloudflared.service | grep -q '^cloudflared.service'; }

if [ -n "$CF_TUNNEL_TOKEN" ]; then
    step "Installing the dashboard tunnel as a service"
    if service_installed; then
        sudo cloudflared service uninstall
    fi
    sudo cloudflared service install "$CF_TUNNEL_TOKEN"
    step "Done"
    echo "In the Cloudflare dashboard, give the tunnel the public hostname"
    echo "  $TUNNEL_HOSTNAME  ->  $ORIGIN"
    exit 0
fi

if [ ! -f "$HOME/.cloudflared/cert.pem" ]; then
    step "Logging in to Cloudflare: open the link below, log in and choose terpstack.dev"
    cloudflared tunnel login
fi

tunnel_id() {
    cloudflared tunnel list --name "$TUNNEL_NAME" --output json 2>/dev/null | python3 -c '
import json, sys
tunnels = [t for t in json.load(sys.stdin) or [] if not t.get("deleted_at") or t["deleted_at"].startswith("0001")]
print(tunnels[0]["id"] if tunnels else "")'
}

TUNNEL_ID="$(tunnel_id)"
if [ -z "$TUNNEL_ID" ]; then
    step "Creating the tunnel $TUNNEL_NAME"
    cloudflared tunnel create "$TUNNEL_NAME"
    TUNNEL_ID="$(tunnel_id)"
fi
[ -n "$TUNNEL_ID" ] || { echo "Could not find the id of tunnel $TUNNEL_NAME."; exit 1; }

CREDS="$HOME/.cloudflared/$TUNNEL_ID.json"
if [ ! -f "$CREDS" ] && ! sudo test -f "$CF_DIR/$TUNNEL_ID.json"; then
    echo "The tunnel $TUNNEL_NAME exists but its credentials file $CREDS is missing on this machine."
    echo "Delete it with 'cloudflared tunnel delete $TUNNEL_NAME' (or pick another TUNNEL_NAME) and run this again."
    exit 1
fi

step "Pointing $TUNNEL_HOSTNAME at the tunnel"
cloudflared tunnel route dns --overwrite-dns "$TUNNEL_NAME" "$TUNNEL_HOSTNAME"

step "Writing $CF_DIR/config.yml ($TUNNEL_HOSTNAME -> $ORIGIN)"
sudo mkdir -p "$CF_DIR"
[ -f "$CREDS" ] && sudo install -m 600 "$CREDS" "$CF_DIR/$TUNNEL_ID.json"
sudo tee "$CF_DIR/config.yml" >/dev/null <<EOF
# Written by apps/lan/install-tunnel.sh; running it again overwrites this file.
tunnel: $TUNNEL_ID
credentials-file: $CF_DIR/$TUNNEL_ID.json

ingress:
  - hostname: $TUNNEL_HOSTNAME
    service: $ORIGIN
  - service: http_status:404
EOF
sudo cloudflared tunnel --config "$CF_DIR/config.yml" ingress validate

step "Running cloudflared as a service"
if service_installed; then
    sudo systemctl restart cloudflared
else
    sudo cloudflared --config "$CF_DIR/config.yml" service install
fi
sudo systemctl enable cloudflared >/dev/null 2>&1 || true

step "Done"
echo "The panel is at https://$TUNNEL_HOSTNAME (DNS can take a minute the first time)."
echo "Check the tunnel with:  systemctl status cloudflared"
