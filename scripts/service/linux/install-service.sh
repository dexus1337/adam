#!/bin/bash

# Default values
SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
EXE_PATH=$(realpath "$SCRIPT_DIR/adam")
SERVICE_NAME="adam.service"
SERVICE_DESC="Adam Application Service"
USER_RUNNING="root"

# Allow passing a custom executable path
if [ ! -z "$1" ]; then
    EXE_PATH=$(realpath "$1")
fi

if [ ! -f "$EXE_PATH" ]; then
    echo -e "\e[33mWarning: Executable not found at $EXE_PATH.\e[0m"
    echo -e "\e[33mPlease provide the correct path as the first argument:\e[0m"
    echo "  ./install-service.sh /path/to/adam"
fi

SERVICE_FILE="/etc/systemd/system/$SERVICE_NAME"

echo "Creating systemd service file at $SERVICE_FILE..."

sudo bash -c "cat > $SERVICE_FILE" << EOF
[Unit]
Description=$SERVICE_DESC
After=network.target

[Service]
Type=simple
ExecStart=$EXE_PATH
Restart=on-failure
RestartSec=5
User=$USER_RUNNING
WorkingDirectory=$(dirname "$EXE_PATH")

[Install]
WantedBy=multi-user.target
EOF

echo "Reloading systemd daemon..."
sudo systemctl daemon-reload

echo "Enabling service to start on boot..."
sudo systemctl enable $SERVICE_NAME

echo "Starting service..."
sudo systemctl start $SERVICE_NAME
sudo systemctl status $SERVICE_NAME --no-pager

echo -e "\e[32mService installed and started successfully!\e[0m"
