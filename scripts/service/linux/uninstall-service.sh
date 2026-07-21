#!/bin/bash

SERVICE_NAME="adam"
SERVICE_FILE="/etc/systemd/system/${SERVICE_NAME}.service"

echo "Stopping service $SERVICE_NAME..."
sudo systemctl stop $SERVICE_NAME 2>/dev/null || true

echo "Disabling service $SERVICE_NAME..."
sudo systemctl disable $SERVICE_NAME 2>/dev/null || true

if [ -f "$SERVICE_FILE" ]; then
    echo "Removing systemd service file at $SERVICE_FILE..."
    sudo rm "$SERVICE_FILE"
else
    echo -e "\e[33mService file $SERVICE_FILE not found.\e[0m"
fi

echo "Reloading systemd daemon..."
sudo systemctl daemon-reload

echo -e "\e[32mService removed successfully!\e[0m"
