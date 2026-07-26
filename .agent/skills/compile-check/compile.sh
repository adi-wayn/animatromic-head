#!/bin/bash
cd "$(dirname "$0")/../../../edge"
echo "Running PlatformIO Build in $(pwd)..."
# Assuming platformio is installed globally or in virtualenv
pio run
