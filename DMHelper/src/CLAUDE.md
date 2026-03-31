# CLAUDE.md — DMHelper

## What this is
Desktop VTT for DMs. C++17/Qt 6. Dual-window: DM view + OpenGL player screen.
Win64 + macOS. All active code is in `DMHelper/src/`. Everything in
`DMHelper-Backend/`, `DMHelperClient/`, `DMHelperShared/`, `DMHelperTest/`
is archived — do not touch.

## Build
```bash
cmake -S DMHelper/src -B build-64_bit-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-64_bit-release --config Release
```
Sources are listed **explicitly** in `CMakeLists.txt` — no globbing. If you
create a `.cpp`/`.h` pair, you must add both to the source list manually.

## Conventions — follow existing code, but note these traps

**Enums** use `TypeName_ValueName` (e.g. `LayerType_Fow`, `CampaignType_Battle`).
This is non-obvious — don't drift to `LayerType::Fow` or plain `FOW`.

**Dirty vs changed signals:** `dirty()` = unsaved data changed.
`changed()` = visual-only. Never emit `dirty()` in constructors or `inputXML()`.

**Serialisation:** Override `createOutputXML()` + `internalOutputXML()`.
Always call the base class implementation. Use `postProcessXML()` for
cross-references — not `inputXML()`.

**Layer subclasses** must implement both interfaces independently:
- DM path: `dmInitialize / dmUninitialize / dmUpdate`
- Player path: `playerGLInitialize / playerGLUninitialize / playerGLPaint`
Neither path may assume the other is active.

## Threading — this is where crashes happen

libvlc decode callbacks (`lockCallback`, `unlockCallback`, `displayCallback`)
run on VLC's internal thread — **never touch Qt GUI objects from them**.
Marshal everything back via:
```cpp
QMetaObject::invokeMethod(obj, ..., Qt::QueuedConnection);
```
`Qt::AA_DontCheckOpenGLContextThreadAffinity` in `main.cpp` is intentional —
`VideoPlayerGLVideo` calls `makeCurrent()` from the VLC thread. Do not remove it.

## New files
If a task requires creating new .cpp/.h files, Claude must:
1. State what files it intends to create and why, before creating them
2. Add them to the explicit source lists in CMakeLists.txt in the same commit
3. Never create a .cpp file without a corresponding CMakeLists.txt update

## GL context rules — critical
OpenGL calls require an active GL context. This is ONLY guaranteed in 
functions with "GL" in their name (playerGLInitialize, playerGLPaint, etc.).

Never make GL calls from:
- Constructors or destructors
- inputXML() or outputXML()  
- Qt signal handlers
- activateObject() / deactivateObject()
- Any function without "GL" in its name

Shader initialisation uses lazy-loading: shaders are created in 
playerGLInitialize() where possible, with a fallback guard at the top 
of playerGLPaint():
  if(_shaderProgramRGBA == 0) createShaders();
This guard is intentional — do not remove it.

## Legacy classes — always use v2

| Wrong | Right |
|---|---|
| `MonsterClass` | `MonsterClassv2` |
| `Character` | `Characterv2` |

Legacy classes remain only for XML file compatibility. New code uses v2 exclusively.

## Disabled feature flags — do not enable without discussion
- `INCLUDE_NETWORK_SUPPORT` — network stack incomplete, gated in `dmconstants.h`
- `LAYERVIDEO_USE_OPENGL` — GPU video path exists but not production-ready

## Files never to modify
- `*.ui` — edit in Qt Designer only, never hand-edit XML
- `*.qrc` — edit manually only with care, never restructure paths
- `vlc32/`, `vlc64/`, `vlcMac/`, `bin-win*/`, `bin-macos/` — pre-built binaries

## Agent workflow
- Branch: `agent/work` — never commit directly to `main`
- Commit after each logical file/unit: `agent: <what changed>`
- After non-trivial changes, run the build command above to verify
- Keep tasks to one subsystem at a time