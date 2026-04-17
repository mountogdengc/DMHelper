# DMHelper Linux Port — Design Spec

## Goal

Port DMHelper to Linux with full feature parity to the Windows version. Distribute as a self-contained AppImage that bundles Qt and VLC libraries so users need no external dependencies.

## Current State

- Qt 6.6 / C++ / qmake — ~95% of code is platform-independent Qt
- Four projects: DMHelper (main), DMHelperClient, DMHelperShared (networking), DMHelperTest
- Platform-specific code limited to VLC integration, file paths, and minor OS conditionals
- GitHub Actions Linux workflow exists but is incomplete (builds, doesn't package)
- Windows and macOS are fully supported with bundled VLC

## Section 1: Platform-Specific Code Audit & Fixes

### VLC Integration
- Window handle types differ per platform: `HWND` (Windows), `NSView*` (Mac), `XID` (Linux/X11)
- VLC player setup code needs a Linux path using X11 window IDs (`XID` from `QWidget::winId()`)
- Wayland users will run under XWayland compatibility (native Wayland VLC support is out of scope)
- All VLC-related `#ifdef` blocks need `Q_OS_LINUX` equivalents

### File Paths
- Linux data directory: `~/.local/share/DMHelper/` (via `QStandardPaths::AppDataLocation`)
- Linux config directory: `~/.config/DMHelper/` (via `QStandardPaths::ConfigLocation`)
- Any hardcoded Windows-style paths must be replaced with `QStandardPaths` calls

### Windows Headers
- `BaseTsd.h` and `ssize_t` — already conditionally included, verify the guards exclude Linux
- Any other Windows-only headers behind `#ifdef Q_OS_WIN`

### OpenGL
- Qt's OpenGL abstraction should work as-is on Linux
- Verify at runtime — may need explicit mesa driver handling

### Application Icon
- Need a 256x256+ PNG icon for Linux desktop integration
- Derive from existing Windows `.ico` or Mac `.icns`

## Section 2: Build System Changes (`.pro` files)

All changes are additive — no modifications to existing Windows or Mac blocks.

### VLC Linking
- Add `unix:!macx:` block to link against `libvlc` and `libvlccore`
- Set include paths and library paths for the bundled or system VLC

### Output Directories
- Linux-specific output path (e.g., `DMHelperLinux/`)

### Compiler Flags
- Existing `-O2` for non-MSVC should work
- May need GCC-specific warning suppressions

### RPATH
- Set `QMAKE_RPATHDIR` to `$ORIGIN/lib` so the binary finds bundled libraries at runtime
- Critical for AppImage self-containment

## Section 3: GitHub Actions CI Pipeline

### Build Environment
- Ubuntu latest LTS
- Dependencies to install:
  - `qt6-base-dev`, `qt6-multimedia-dev`, `qt6-opengl-dev`
  - `qt6-networkauth-dev`, `qt6-imageformats-dev`
  - `libvlc-dev`, `libvlccore-dev`
  - `libgl1-mesa-dev`
  - Other Qt 6 modules as needed

### Build Steps
1. `qmake6` for each project
2. `make -j$(nproc)` for each project

### AppImage Packaging
- Use `linuxdeploy` with the Qt plugin
- Bundle DMHelper binary, Qt libraries/plugins, VLC libraries/plugins
- Include `.desktop` file and icon
- Produce a single `.AppImage` file

### Artifacts
- Upload AppImage as GitHub Actions artifact
- Optionally attach to GitHub Releases alongside Windows/Mac builds

## Section 4: AppImage Structure & Desktop Integration

### AppImage Contents
```
DMHelper.AppImage
├── AppRun (entry point)
├── DMHelper.desktop
├── DMHelper.png
├── usr/
│   ├── bin/
│   │   └── DMHelper
│   ├── lib/
│   │   ├── libQt6*.so
│   │   ├── libvlc.so*
│   │   ├── libvlccore.so*
│   │   └── vlc/
│   │       └── plugins/
│   └── plugins/
│       └── (Qt plugins)
```

### Desktop Entry File
```ini
[Desktop Entry]
Type=Application
Name=DMHelper
Comment=Dungeons & Dragons Campaign Manager
Exec=DMHelper
Icon=DMHelper
Categories=Game;RolePlaying;
```

### Runtime Configuration
- `VLC_PLUGIN_PATH` set in AppRun to point to bundled VLC plugins
- User data: `~/.local/share/DMHelper/`
- Config: `~/.config/DMHelper/`

## Section 5: Testing & Verification

### Build Verification
- All four projects compile on Linux without errors
- Windows and Mac CI builds remain green (regression check)

### Runtime Testing (on VM)
- Application starts and displays ribbon UI
- Campaign creation and loading
- Battle map rendering (OpenGL)
- Character/monster sheet display
- VLC video playback
- Audio playback (file, URL)
- Network features (if applicable)

### AppImage Testing
- Run on clean system with no Qt or VLC installed
- Verify fully self-contained — no missing library errors

## Out of Scope
- CMake migration (future consideration, not needed for this port)
- Wayland-native support (X11/XWayland is sufficient for now)
- Linux-specific features beyond parity
- Flatpak or `.deb` packaging (AppImage only)
