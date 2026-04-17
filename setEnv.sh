#!/bin/bash
# Script to generate .env file with current user's UID/GID

cat > .env << EOF
# Auto-generated environment variables for dev container
USER_ID=$(id -u)
GROUP_ID=$(id -g)
USER=$(whoami)
XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR:-/run/user/$(id -u)}
DISPLAY=${DISPLAY:-:0}
WAYLAND_DISPLAY=${WAYLAND_DISPLAY:-wayland-0}
EOF

echo ".env file created with:"
cat .env
