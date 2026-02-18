#!/bin/bash

# Script to install dependencies for SimulIDE on Ubuntu/Debian
# Usage: sudo ./scripts/setup-linux.sh

echo "=== LINUX DEPENDENCY SETUP START ==="

# Update package list
sudo apt-get update

# Install build tools
echo "Installing build tools..."
sudo apt-get install -y build-essential git cmake

# Install Qt6 dependencies
echo "Installing Qt6 dependencies..."
sudo apt-get install -y \
    qt6-base-dev \
    qt6-multimedia-dev \
    qt6-serialport-dev \
    qt6-svg-dev \
    qt6-tools-dev-tools \
    libqt6multimedia6 \
    libqt6serialport6 \
    libqt6svg6 \
    libqt6xml6

# Install LibElf
echo "Installing libelf..."
sudo apt-get install -y libelf-dev

echo "=== LINUX DEPENDENCY SETUP DONE ==="
echo "You can now run ./scripts/build-linux.sh to build the project."
