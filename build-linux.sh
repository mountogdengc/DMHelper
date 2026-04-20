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
  libgl1-mesa-dev \
  build-essential \
  cmake \
  yt-dlp

REPO_ROOT="$(cd "$(dirname "$0")" && git rev-parse --show-toplevel)"

echo "=== Building DMHelper ==="
cd "$REPO_ROOT"
rm -rf build-release
cmake -S DMHelper/src -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --config Release -j$(nproc)

echo "=== Deploying resources ==="
cd build-release
cp -r "$REPO_ROOT/DMHelper/src/resources" .
cp -r "$REPO_ROOT/DMHelper/src/doc" .
# Bestiary files go in resources/ where getAbsoluteTemplateFile() looks
cp "$REPO_ROOT/DMHelper/src/bestiary/"*.xml resources/

# Deploy bundled VLC 4 libraries
echo "=== Deploying VLC 4 libraries ==="
mkdir -p lib
cp "$REPO_ROOT/DMHelper/src/vlclinux/libvlc.so.12.0.0" lib/
cp "$REPO_ROOT/DMHelper/src/vlclinux/libvlccore.so.9.0.0" lib/
cd lib
ln -sf libvlc.so.12.0.0 libvlc.so.12
ln -sf libvlc.so.12.0.0 libvlc.so
ln -sf libvlccore.so.9.0.0 libvlccore.so.9
ln -sf libvlccore.so.9.0.0 libvlccore.so
cd ..

echo "=== Build complete! ==="
echo "Run with: LD_LIBRARY_PATH=$(pwd)/lib $(pwd)/DMHelper"
