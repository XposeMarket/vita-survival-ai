#!/bin/bash
# Build script for Vita Survival AI

set -e

echo "==================================="
echo " Vita Survival AI - Build Script"
echo "==================================="

# Check for VITASDK
if [ -z "$VITASDK" ]; then
    echo "Error: VITASDK environment variable not set"
    echo "Please install VitaSDK and set VITASDK to the installation path"
    exit 1
fi

echo "Using VitaSDK: $VITASDK"

# Create build directory
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build
echo "Building..."
make -j$(nproc)

echo ""
echo "==================================="
echo " Build Complete!"
echo "==================================="
echo "Output: SurvivalAI.vpk"
echo ""
echo "To install:"
echo "1. Copy SurvivalAI.vpk to your Vita"
echo "2. Install using VitaShell"
echo "3. Data directory: ux0:data/survivalkit/"
echo "==================================="
