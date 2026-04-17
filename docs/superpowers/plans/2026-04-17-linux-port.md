# DMHelper Linux Port Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Port DMHelper to Linux with full feature parity and distribute as a self-contained AppImage bundling Qt and VLC.

**Architecture:** The codebase is Qt 6 / C++ with ~95% platform-independent code. The port requires: (1) adding Linux blocks alongside existing Windows/Mac conditionals in ~10 source files, (2) adding Linux VLC linking and compiler config in the .pro build file, (3) rewriting the broken GitHub Actions Linux workflow, and (4) creating AppImage packaging with bundled VLC.

**Tech Stack:** Qt 6.6, C++11, qmake, libvlc, OpenGL, GitHub Actions, linuxdeploy (AppImage tooling)

**Spec:** `docs/superpowers/specs/2026-04-17-linux-port-design.md`

---

### Task 1: Add Linux VLC Linking and Compiler Config to DMHelper.pro

**Files:**
- Modify: `DMHelper/src/DMHelper.pro:838-872` (after the macx VLC block)

This task adds the missing Linux-specific build configuration. Currently only Windows and Mac blocks exist.

- [ ] **Step 1: Add Linux compiler settings after the Mac release block (after line 837)**

Open `DMHelper/src/DMHelper.pro` and add the following after the `macx:CONFIG(release)` block (after line 837, before the `# link to libvlc` comment):

```qmake
unix:!macx:CONFIG(debug, debug|release) {
    message("Linux Debug build detected")
    QMAKE_CXXFLAGS += -g
}

unix:!macx:CONFIG(release, debug|release) {
    message("Linux Release build detected")
    QMAKE_CXXFLAGS += -O2
    DEFINES += NDEBUG
}
```

- [ ] **Step 2: Add Linux VLC linking block after the macx VLC block (after line 871)**

Add the following after the closing `}` of the `macx` VLC block:

```qmake
unix:!macx {
    message("Linux 64-bit VLC")
    INCLUDEPATH += /usr/include
    INCLUDEPATH += /usr/include/vlc
    LIBS += -lvlc
    QMAKE_RPATHDIR += $ORIGIN/lib
}
```

- [ ] **Step 3: Verify the .pro file is valid by checking syntax**

Run from the repo root:
```bash
cd DMHelper/src && grep -n "unix:!macx" DMHelper.pro
```
Expected: 3 or more matches showing the new Linux blocks.

- [ ] **Step 4: Commit**

```bash
git add DMHelper/src/DMHelper.pro
git commit -m "build: add Linux compiler settings and VLC linking to DMHelper.pro"
```

---

### Task 2: Fix VLC API Conditionals for Linux

**Files:**
- Modify: `DMHelper/src/dmh_vlc.h:7-10`
- Modify: `DMHelper/src/videoplayer.cpp:611-623`
- Modify: `DMHelper/src/videoplayerglplayer.cpp:507-519`
- Modify: `DMHelper/src/videoplayerglscreenshot.cpp:186-201`
- Modify: `DMHelper/src/audiotrackyoutube.cpp:333-343`

The current code uses `#if defined(Q_OS_WIN64) || defined(Q_OS_MAC)` to choose between VLC 4.x API (new, no instance arg for `libvlc_media_new_path`) and VLC 3.x API (old, requires instance arg). On Linux, we'll use the system VLC which on Ubuntu is VLC 3.x, so Linux should use the `else` branch (old API). The existing `#else` branches already handle this correctly — Linux falls through to the old API. **No source code changes needed for the API conditionals.**

However, the `dmh_vlc.h` header needs a fix to ensure it doesn't try to include Windows headers on Linux.

- [ ] **Step 1: Verify dmh_vlc.h Windows guard is correct**

Read `DMHelper/src/dmh_vlc.h` lines 1-15. The existing code is:

```cpp
#ifdef Q_OS_WIN
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#endif
```

This is correct — the `#ifdef Q_OS_WIN` guard already excludes Linux. Linux has `ssize_t` natively in `<sys/types.h>`. No change needed.

- [ ] **Step 2: Verify VLC API fallthrough is correct for all files**

Run:
```bash
cd DMHelper/src && grep -n "Q_OS_WIN64.*Q_OS_MAC" videoplayer.cpp videoplayerglplayer.cpp videoplayerglscreenshot.cpp audiotrackyoutube.cpp
```

Expected: Each file shows `#if defined(Q_OS_WIN64) || defined(Q_OS_MAC)` with the new API, and the `#else` branch uses the old API (with instance parameter). Linux will use the `#else` path, which is correct for VLC 3.x.

- [ ] **Step 3: Commit (no-op — document verification)**

No source changes needed. The existing conditionals already handle Linux correctly via the `#else` fallthrough. Record this in a commit message:

```bash
git commit --allow-empty -m "docs: verify VLC API conditionals handle Linux correctly via #else fallthrough"
```

---

### Task 3: Fix OS Logging in mainwindow.cpp

**Files:**
- Modify: `DMHelper/src/mainwindow.cpp:184-188`

The OS logging currently reports "MacOS" or "Windows" but never "Linux".

- [ ] **Step 1: Update the OS detection logging**

In `DMHelper/src/mainwindow.cpp`, find the block at line 184:

```cpp
#ifdef Q_OS_MAC
    qDebug() << "[MainWindow]     OS: MacOS";
#else
    qDebug() << "[MainWindow]     OS: Windows";
#endif
```

Replace with:

```cpp
#ifdef Q_OS_MAC
    qDebug() << "[MainWindow]     OS: MacOS";
#elif defined(Q_OS_LINUX)
    qDebug() << "[MainWindow]     OS: Linux";
#else
    qDebug() << "[MainWindow]     OS: Windows";
#endif
```

- [ ] **Step 2: Commit**

```bash
git add DMHelper/src/mainwindow.cpp
git commit -m "fix: add Linux OS identification in startup logging"
```

---

### Task 4: Remove Windows Path Separator Conversion on Linux

**Files:**
- Modify: `DMHelper/src/videoplayer.cpp:61-63`
- Modify: `DMHelper/src/videoplayerglplayer.cpp:44-46`
- Modify: `DMHelper/src/videoplayerglscreenshot.cpp:183-185`

These files convert forward slashes to backslashes for VLC on Windows. The `#ifdef Q_OS_WIN` guard already excludes Linux, so this is a verification task.

- [ ] **Step 1: Verify all three files have the Q_OS_WIN guard**

Run:
```bash
cd DMHelper/src && grep -B1 -A1 'replace.*\\\\\\\\' videoplayer.cpp videoplayerglplayer.cpp videoplayerglscreenshot.cpp
```

Expected: Each occurrence is inside `#ifdef Q_OS_WIN`. Linux will keep forward slashes (correct).

- [ ] **Step 2: Commit (no-op — verification only)**

No changes needed. The guards already exclude Linux.

```bash
git commit --allow-empty -m "docs: verify Windows path separator code is guarded from Linux"
```

---

### Task 5: Create Linux Application Icon

**Files:**
- Create: `DMHelper/src/data/dmhelper.png` (256x256 application icon for Linux/AppImage)

- [ ] **Step 1: Extract a PNG from the existing .ico file**

The Windows icon `DMHelper/src/dmhelper.ico` contains multiple sizes. Extract the largest:

```bash
cd DMHelper/src
# Use ImageMagick if available, otherwise use the existing PNG from resources
# The resources.qrc already references data/dmhelper.png - check if it exists
ls -la data/dmhelper.png
```

If `data/dmhelper.png` already exists (it's referenced in `resources.qrc`), verify it's at least 256x256:

```bash
file data/dmhelper.png
```

- [ ] **Step 2: If the icon is too small, extract from .ico**

If the existing PNG is too small (less than 128x128), use ImageMagick on your VM:

```bash
convert dmhelper.ico[0] -resize 256x256 data/dmhelper_256.png
```

Otherwise, the existing `data/dmhelper.png` is sufficient for the AppImage.

- [ ] **Step 3: Commit**

```bash
git add DMHelper/src/data/dmhelper*.png
git commit -m "assets: ensure Linux-compatible application icon exists"
```

---

### Task 6: Create Desktop Entry File for AppImage

**Files:**
- Create: `DMHelper/src/data/linux/DMHelper.desktop`

- [ ] **Step 1: Create the linux directory**

```bash
mkdir -p DMHelper/src/data/linux
```

- [ ] **Step 2: Create the .desktop file**

Create `DMHelper/src/data/linux/DMHelper.desktop` with:

```ini
[Desktop Entry]
Type=Application
Name=DMHelper
GenericName=D&D Campaign Manager
Comment=Dungeons & Dragons campaign management tool with battle maps, character sheets, and multimedia
Exec=DMHelper
Icon=DMHelper
Categories=Game;RolePlaying;
Terminal=false
StartupNotify=true
```

- [ ] **Step 3: Verify the desktop file format**

```bash
# Check no trailing whitespace or BOM
file DMHelper/src/data/linux/DMHelper.desktop
cat -A DMHelper/src/data/linux/DMHelper.desktop | head -3
```

Expected: ASCII text, no `^M` (carriage returns), no BOM.

- [ ] **Step 4: Commit**

```bash
git add DMHelper/src/data/linux/DMHelper.desktop
git commit -m "feat: add freedesktop .desktop entry for Linux AppImage"
```

---

### Task 7: Create AppRun Script for VLC Plugin Path

**Files:**
- Create: `DMHelper/src/data/linux/AppRun.sh`

The AppImage needs a wrapper script to set `VLC_PLUGIN_PATH` before launching the binary so bundled VLC plugins are found.

- [ ] **Step 1: Create the AppRun script**

Create `DMHelper/src/data/linux/AppRun.sh` with:

```bash
#!/bin/bash
HERE="$(dirname "$(readlink -f "${0}")")"
export VLC_PLUGIN_PATH="${HERE}/usr/lib/vlc/plugins"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
exec "${HERE}/usr/bin/DMHelper" "$@"
```

- [ ] **Step 2: Make it executable**

```bash
chmod +x DMHelper/src/data/linux/AppRun.sh
```

- [ ] **Step 3: Commit**

```bash
git add DMHelper/src/data/linux/AppRun.sh
git commit -m "feat: add AppRun script for Linux AppImage VLC plugin path setup"
```

---

### Task 8: Rewrite GitHub Actions Linux Workflow

**Files:**
- Modify: `.github/workflows/dmh-build-linux.yml` (full rewrite)

The existing workflow is broken — it uses Windows tools (qmake.exe, JOM, backslash paths, PowerShell) on a Linux runner.

- [ ] **Step 1: Rewrite the workflow file**

Replace the entire contents of `.github/workflows/dmh-build-linux.yml` with:

```yaml
name: DMHelper CI Linux Workflow

on:
  workflow_dispatch:

env:
  SOURCE_DIR: ${{ github.workspace }}
  QT_VERSION: 6.6.0
  APPIMAGE_NAME: DMHelper-x86_64.AppImage

jobs:
  build-DMH-linux:
    runs-on: ubuntu-22.04

    steps:
    - name: Install system dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y \
          libvlc-dev \
          libvlccore-dev \
          vlc-plugin-base \
          vlc-plugin-video-output \
          libgl1-mesa-dev \
          libxkbcommon-dev \
          libxkbcommon-x11-0 \
          libxcb-xinerama0 \
          libxcb-cursor0 \
          fuse \
          libfuse2

    - name: Install Qt
      uses: jurplel/install-qt-action@v3
      with:
        aqtversion: '==3.1.*'
        version: ${{ env.QT_VERSION }}
        host: 'linux'
        target: 'desktop'
        arch: 'gcc_64'
        modules: 'qtimageformats qtmultimedia qtnetworkauth'
        archives: 'qttranslations qttools qtsvg qtbase icu'

    - name: Check out the DMHelper source tree
      uses: actions/checkout@v4

    - name: Build DMHelper
      run: |
        cd DMHelper
        mkdir build-release
        cd build-release
        qmake ../src/DMHelper.pro CONFIG+=release
        make -j$(nproc)

    - name: Prepare AppDir structure
      run: |
        mkdir -p AppDir/usr/bin
        mkdir -p AppDir/usr/lib/vlc/plugins
        mkdir -p AppDir/usr/share/applications
        mkdir -p AppDir/usr/share/icons/hicolor/256x256/apps

        # Copy the built binary
        cp DMHelper/build-release/DMHelper AppDir/usr/bin/

        # Copy resources, bestiary, and doc directories
        cp -r DMHelper/src/resources AppDir/usr/bin/resources
        cp -r DMHelper/src/bestiary AppDir/usr/bin/bestiary
        cp -r DMHelper/src/doc AppDir/usr/bin/doc

        # Copy VLC libraries
        cp -P /usr/lib/x86_64-linux-gnu/libvlc.so* AppDir/usr/lib/
        cp -P /usr/lib/x86_64-linux-gnu/libvlccore.so* AppDir/usr/lib/

        # Copy VLC plugins
        cp -r /usr/lib/x86_64-linux-gnu/vlc/plugins/* AppDir/usr/lib/vlc/plugins/

        # Copy desktop file and icon
        cp DMHelper/src/data/linux/DMHelper.desktop AppDir/usr/share/applications/
        cp DMHelper/src/data/linux/DMHelper.desktop AppDir/
        cp DMHelper/src/data/dmhelper.png AppDir/usr/share/icons/hicolor/256x256/apps/DMHelper.png
        cp DMHelper/src/data/dmhelper.png AppDir/DMHelper.png

        # Copy AppRun
        cp DMHelper/src/data/linux/AppRun.sh AppDir/AppRun
        chmod +x AppDir/AppRun

    - name: Download linuxdeploy
      run: |
        wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
        wget -q https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
        chmod +x linuxdeploy-x86_64.AppImage
        chmod +x linuxdeploy-plugin-qt-x86_64.AppImage

    - name: Create AppImage
      run: |
        export QMAKE=$(which qmake)
        ./linuxdeploy-x86_64.AppImage \
          --appdir AppDir \
          --plugin qt \
          --output appimage \
          --desktop-file AppDir/DMHelper.desktop \
          --icon-file AppDir/DMHelper.png \
          --executable AppDir/usr/bin/DMHelper

    - name: Upload AppImage artifact
      uses: actions/upload-artifact@v4
      with:
        name: ${{ env.APPIMAGE_NAME }}
        path: DMHelper-x86_64.AppImage
```

- [ ] **Step 2: Verify YAML syntax**

```bash
python3 -c "import yaml; yaml.safe_load(open('.github/workflows/dmh-build-linux.yml'))" 2>&1 && echo "YAML OK"
```

Expected: `YAML OK`

- [ ] **Step 3: Commit**

```bash
git add .github/workflows/dmh-build-linux.yml
git commit -m "ci: rewrite Linux workflow with proper build tools and AppImage packaging"
```

---

### Task 9: Test Local Linux Build on VM

This task is performed manually on your Linux VM.

- [ ] **Step 1: Install build dependencies on your VM**

```bash
sudo apt-get update
sudo apt-get install -y \
  qt6-base-dev \
  qt6-multimedia-dev \
  qt6-tools-dev \
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
```

- [ ] **Step 2: Clone the repo and build**

```bash
git clone https://github.com/mountogdengc/DMHelper.git
cd DMHelper/DMHelper
mkdir build-release
cd build-release
qmake6 ../src/DMHelper.pro CONFIG+=release
make -j$(nproc)
```

Expected: Build completes with 0 errors. Warnings are acceptable.

- [ ] **Step 3: Run the application**

```bash
./DMHelper
```

Expected: The DMHelper window opens with the ribbon UI. Create a test campaign, open a battle map, verify rendering.

- [ ] **Step 4: Test VLC playback**

Load a campaign with video or audio content. Verify:
- Video plays in battle maps
- Audio tracks play (file-based)
- YouTube audio works (if network available)

- [ ] **Step 5: Fix any build or runtime errors**

If there are compiler errors, fix them and update the relevant source files. Common issues to watch for:
- Missing Qt module in the .pro `QT +=` line — add the module
- Missing `#include` for Linux-specific headers — add behind `#ifdef Q_OS_LINUX`
- OpenGL function not found — may need `#include <GL/gl.h>` on Linux

- [ ] **Step 6: Commit any fixes**

```bash
git add -A
git commit -m "fix: resolve Linux build/runtime issues found during testing"
```

---

### Task 10: Test AppImage Packaging Locally

- [ ] **Step 1: Install linuxdeploy on your VM**

```bash
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage linuxdeploy-plugin-qt-x86_64.AppImage
```

- [ ] **Step 2: Create the AppDir structure**

From the repo root:

```bash
mkdir -p AppDir/usr/bin
mkdir -p AppDir/usr/lib/vlc/plugins
mkdir -p AppDir/usr/share/applications
mkdir -p AppDir/usr/share/icons/hicolor/256x256/apps

cp DMHelper/build-release/DMHelper AppDir/usr/bin/
cp -r DMHelper/src/resources AppDir/usr/bin/resources
cp -r DMHelper/src/bestiary AppDir/usr/bin/bestiary
cp -r DMHelper/src/doc AppDir/usr/bin/doc

cp -P /usr/lib/x86_64-linux-gnu/libvlc.so* AppDir/usr/lib/
cp -P /usr/lib/x86_64-linux-gnu/libvlccore.so* AppDir/usr/lib/
cp -r /usr/lib/x86_64-linux-gnu/vlc/plugins/* AppDir/usr/lib/vlc/plugins/

cp DMHelper/src/data/linux/DMHelper.desktop AppDir/usr/share/applications/
cp DMHelper/src/data/linux/DMHelper.desktop AppDir/
cp DMHelper/src/data/dmhelper.png AppDir/DMHelper.png
cp DMHelper/src/data/dmhelper.png AppDir/usr/share/icons/hicolor/256x256/apps/DMHelper.png

cp DMHelper/src/data/linux/AppRun.sh AppDir/AppRun
chmod +x AppDir/AppRun
```

- [ ] **Step 3: Build the AppImage**

```bash
export QMAKE=$(which qmake6)
./linuxdeploy-x86_64.AppImage \
  --appdir AppDir \
  --plugin qt \
  --output appimage \
  --desktop-file AppDir/DMHelper.desktop \
  --icon-file AppDir/DMHelper.png \
  --executable AppDir/usr/bin/DMHelper
```

Expected: Produces `DMHelper-x86_64.AppImage` in the current directory.

- [ ] **Step 4: Test the AppImage**

```bash
chmod +x DMHelper-x86_64.AppImage
./DMHelper-x86_64.AppImage
```

Expected: DMHelper launches without requiring system Qt or VLC libraries. Full feature parity with the direct build.

- [ ] **Step 5: Commit any packaging fixes**

```bash
git add -A
git commit -m "fix: resolve AppImage packaging issues found during testing"
```

---

### Task 11: Run GitHub Actions CI and Verify

- [ ] **Step 1: Push all changes to the repository**

```bash
git push origin main
```

- [ ] **Step 2: Trigger the Linux workflow**

Go to GitHub > Actions > "DMHelper CI Linux Workflow" > "Run workflow", or:

```bash
gh workflow run dmh-build-linux.yml
```

- [ ] **Step 3: Monitor the build**

```bash
gh run watch
```

Expected: All steps pass. The AppImage artifact is uploaded.

- [ ] **Step 4: Download and test the CI-built AppImage**

Download the artifact from the GitHub Actions run and test on your VM.

- [ ] **Step 5: Verify Windows and Mac builds still work**

Trigger the Windows and Mac workflows to confirm no regressions:

```bash
gh workflow run dmh-build-windows.yml
gh workflow run dmh-build-mac.yml
```

Expected: Both complete successfully with no new errors.

- [ ] **Step 6: Commit any CI fixes**

If the workflow fails, fix the YAML and push again:

```bash
git add .github/workflows/dmh-build-linux.yml
git commit -m "ci: fix Linux workflow issues found during CI run"
git push origin main
```
