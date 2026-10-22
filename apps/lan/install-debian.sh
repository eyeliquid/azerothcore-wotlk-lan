#!/usr/bin/env bash
#
# Installs the LAN server on Debian 12+ for the user debwow, server address 192.168.2.34.
# Run it from the cloned repository as debwow (it asks for sudo):
#
#   bash apps/lan/install-debian.sh
#
# Running it again updates: it rebuilds, reinstalls and keeps the existing .conf files.

set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
INSTALL_DIR="${INSTALL_DIR:-$HOME/azeroth-server}"
JOBS="${JOBS:-$(nproc)}"
CLIENT_DATA_VERSION="v20.0"
MYSQL_APT_CONFIG_VERSION="0.8.36-1"

step() { echo; echo "==> $*"; }

if [ "$EUID" -eq 0 ]; then
    echo "Run this as your normal user (debwow), not as root. It uses sudo where needed."
    exit 1
fi

step "Installing build tools and libraries"
sudo apt-get update -y
sudo apt-get install -y git cmake make clang g++ gcc ccache \
    libssl-dev libbz2-dev libreadline-dev libncurses-dev \
    libboost-all-dev unzip curl wget gnupg lsb-release tmux

if ! dpkg -s mysql-server >/dev/null 2>&1; then
    step "Installing MySQL 8 (Oracle repository; Debian's MariaDB does not work)"
    tmp="$(mktemp -d)"
    wget -q -O "$tmp/mysql-apt-config.deb" \
        "https://dev.mysql.com/get/mysql-apt-config_${MYSQL_APT_CONFIG_VERSION}_all.deb"
    sudo DEBIAN_FRONTEND=noninteractive dpkg -i "$tmp/mysql-apt-config.deb"
    rm -rf "$tmp"
    sudo apt-get update -y
    sudo DEBIAN_FRONTEND=noninteractive apt-get install -y mysql-server libmysqlclient-dev
fi
sudo systemctl enable --now mysql

step "Creating the databases and the acore user"
sudo mysql < "$REPO_DIR/data/sql/create/create_mysql.sql"
sudo mysql -e "CREATE DATABASE IF NOT EXISTS acore_playerbots DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
GRANT ALL PRIVILEGES ON acore_playerbots.* TO 'acore'@'localhost'; FLUSH PRIVILEGES;"

step "Building (this takes a while; set JOBS=2 if the machine runs out of memory)"
mkdir -p "$REPO_DIR/build"
cd "$REPO_DIR/build"
cmake .. -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DSCRIPTS=static -DMODULES=static -DTOOLS_BUILD=none
make -j"$JOBS"
make install

DATA_DIR="$INSTALL_DIR/data"
if [ ! -d "$DATA_DIR/maps" ]; then
    step "Downloading the client data ($CLIENT_DATA_VERSION)"
    mkdir -p "$DATA_DIR"
    curl -L -o "$DATA_DIR/data.zip" \
        "https://github.com/wowgaming/client-data/releases/download/$CLIENT_DATA_VERSION/data.zip"
    unzip -q -o "$DATA_DIR/data.zip" -d "$DATA_DIR"
    rm "$DATA_DIR/data.zip"
fi

step "Creating config files that don't exist yet"
cd "$INSTALL_DIR/etc"
for dist in *.conf.dist modules/*.conf.dist; do
    [ -e "$dist" ] || continue
    conf="${dist%.dist}"
    [ -e "$conf" ] || cp "$dist" "$conf"
done
# The client data lives in this install, wherever INSTALL_DIR points
sed -i "s|^DataDir *=.*|DataDir = \"$DATA_DIR\"|" worldserver.conf

if command -v ufw >/dev/null 2>&1 && sudo ufw status | grep -q "Status: active"; then
    step "Opening ports 3724 and 8085 in the firewall"
    sudo ufw allow 3724/tcp
    sudo ufw allow 8085/tcp
fi

step "Done"
echo "Start the servers with:  bash $REPO_DIR/apps/lan/start.sh"
echo "The first start sets up the databases and the bot accounts, which takes several minutes."
echo "Once the worldserver is up, add the web admin panel and its tunnel (https://ermewow.terpstack.dev) with:"
echo "  bash $REPO_DIR/apps/lan/install-webadmin.sh && bash $REPO_DIR/apps/lan/install-tunnel.sh"
