#!/usr/bin/env bash
#
# Starts authserver and worldserver in a tmux session called "wow".
#   bash apps/lan/start.sh          start
#   tmux attach -t wow              open the consoles (Ctrl+B then D to leave them running)
#   bash apps/lan/start.sh stop     shut both down

set -euo pipefail

INSTALL_DIR="${INSTALL_DIR:-$HOME/azeroth-server}"
BIN="$INSTALL_DIR/bin"
SESSION="wow"

if [ "${1:-}" = "stop" ]; then
    if tmux has-session -t "$SESSION" 2>/dev/null; then
        tmux send-keys -t "$SESSION:world" "server shutdown 1" Enter
        tmux send-keys -t "$SESSION:auth" C-c
        sleep 15
        tmux kill-session -t "$SESSION" 2>/dev/null || true
    fi
    echo "Stopped."
    exit 0
fi

if tmux has-session -t "$SESSION" 2>/dev/null; then
    echo "Already running. Open the consoles with: tmux attach -t $SESSION"
    exit 0
fi

# Each server is started again when it crashes or restarts; a normal shutdown ends it
tmux new-session -d -s "$SESSION" -n auth -c "$BIN"
tmux send-keys -t "$SESSION:auth" "until ./authserver; do sleep 5; done" Enter
sleep 2
tmux new-window -t "$SESSION" -n world -c "$BIN"
tmux send-keys -t "$SESSION:world" "until ./worldserver; do sleep 5; done" Enter

echo "Started. Open the consoles with: tmux attach -t $SESSION"
