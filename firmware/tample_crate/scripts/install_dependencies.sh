#!/bin/bash

set -e

echo "Updating system packages..."
sudo apt update && sudo apt upgrade -y

sudo apt-get install imagemagick

echo "Installing snapd..."
sudo apt install -y snapd

echo "Enabling snapd service..."
sudo systemctl enable --now snapd.socket

# Needed for classic confinement
echo "Creating symbolic link for /snap if not present..."
sudo ln -s /var/lib/snapd/snap /snap || true

echo "Installing Node.js via snap (classic mode)..."
sudo snap install node --classic

echo "Verifying Node.js installation..."
node -v
npm -v

echo "✅ Node.js installation via Snap is complete!"


echo "Installing pm2 ..."
npm install pm2 -g