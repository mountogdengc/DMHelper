#!/bin/bash
set -e

echo "=== Installing dependencies ==="
sudo apt-get update
sudo apt-get install -y \
  cmake \
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
  pkg-config \
  build-essential

REPO_ROOT="$(cd "$(dirname "$0")" && git rev-parse --show-toplevel)"

echo "=== Building DMHelper ==="
cd "$REPO_ROOT"
cmake -S "$REPO_ROOT/DMHelper/src" -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --config Release -j"$(nproc)"

echo "=== Deploying resources ==="
cd build-release
cp -r "$REPO_ROOT/DMHelper/src/resources" .
cp -r "$REPO_ROOT/DMHelper/src/doc" .
# Bestiary files go in resources/ where getAbsoluteTemplateFile() looks
cp "$REPO_ROOT/DMHelper/src/bestiary/"*.xml resources/

echo "=== Build complete! ==="
echo "Run with: $(pwd)/DMHelper"
