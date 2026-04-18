#!/bin/bash
set -e

echo "=== Installing dependencies ==="
sudo apt-get update
sudo apt-get install -y \
  qt6-base-dev \
  qt6-multimedia-dev \
  qt6-tools-dev \
  qt6-tools-dev-tools \
  qt6-l10n-tools \
  qt6-image-formats-plugins \
  libqt6opengl6-dev \
  libqt6openglwidgets6 \
  libqt6uitools6 \
  libqt6networkauth6-dev \
  libvlc-dev \
  libvlccore-dev \
  vlc-plugin-base \
  libgl1-mesa-dev \
  build-essential

echo "=== Building DMHelper ==="
cd "$(dirname "$0")/DMHelper"
rm -rf build-release
mkdir build-release
cd build-release
qmake6 ../src/DMHelper.pro CONFIG+=release
make -j$(nproc)

echo "=== Build complete! ==="
echo "Run with: $(pwd)/DMHelper"
