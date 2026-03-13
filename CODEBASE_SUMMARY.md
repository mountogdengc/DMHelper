# DMHelper — Codebase Technical Summary

> **Scope:** `DMHelper/src/` only. The directories `DMHelper-Backend`, `DMHelperClient`, `DMHelperShared`, and `DMHelperTest` are deprecated and archived; their content is either compiled directly into the main executable (shared sources) or no longer maintained.

---

## 1. Project Purpose and Overview

DMHelper is a desktop **virtual tabletop (VTT)** application for Dungeon Masters running tabletop RPG sessions. It presents a **dual-window architecture**: a feature-rich *DM view* (planning, campaign tree, bestiary, spell book, combat management) and a full-screen *Player view* rendered with hardware-accelerated OpenGL for displaying maps, battle grids, tokens, video effects, and overlays to the players.

The application is written in **C++17 with Qt 6** and targets Windows (MSVC 64-bit) and macOS. It supports D&D 5e and 2e rule-sets via a pluggable initiative/ruleset system, and integrates **libvlc** for GPU-accelerated video playback on the player screen.

**Current version:** `3.7.1` (app), campaign file format `2.4`, bestiary `2.3` — see `DMHelper/src/dmversion.h`.

---

## 2. Directory Structure

```
DMHelper/                         ← Repository root
├── DMHelper/                     ← Main application
│   ├── src/                      ← ALL active source code (C++, .ui, .qrc, CMakeLists.txt)
│   │   ├── CMakeLists.txt        ← Primary build file
│   │   ├── main.cpp              ← Entry point
│   │   ├── dmconstants.h         ← Global enums and constants (DMHelper namespace)
│   │   ├── dmversion.h           ← Version numbers for app, campaign, bestiary, spellbook
│   │   ├── dmh_vlc.h/cpp         ← libvlc singleton wrapper
│   │   ├── dmh_opengl.h/cpp      ← OpenGL debug macro helpers
│   │   ├── resources.qrc         ← Qt resource file (images, shaders, data)
│   │   ├── data/                 ← Bundled data (SRD bestiary, quick-ref, templates)
│   │   ├── vlc32/, vlc64/        ← Windows VLC headers + import lib (32/64-bit)
│   │   ├── vlcMac/               ← macOS VLC headers + lib
│   │   ├── bin-win32/, bin-win64/, bin-macos/  ← Pre-built VLC DLLs / dylibs for deploy
│   │   └── installer/            ← Qt Installer Framework config
│   ├── buildanddeploy_msvc64_cmake.ps1/.bat  ← Windows full build+deploy script
│   └── buildanddeploy_macos_cmake.sh          ← macOS full build+deploy script
│
├── DMHelperShared/               ← Archived; shared network/login library
│   ├── inc/                      ← Public headers (dmhlogon.h, dmhnetworkmanager.h, …)
│   └── src/                      ← Implementation — compiled directly into DMHelper exe
│
├── DMHelper-Backend/             ← Archived server backend
├── DMHelperClient/               ← Archived client app
└── DMHelperTest/                 ← Archived test project
```

### What lives where and why

| Path pattern | Purpose |
|---|---|
| `campaign*.h/cpp` | Data model: campaign document, tree, persistence |
| `battle*.h/cpp` | Combat encounter UI and model |
| `layer*.h/cpp` | Composable rendering layers (image, FoW, tokens, video, draw, …) |
| `publishgl*.h/cpp` | OpenGL player-window rendering pipeline |
| `videoplayer*.h/cpp` | libvlc-backed video playback (CPU + GPU paths) |
| `audiotrack*.h/cpp`, `audioplayer.h/cpp` | Audio subsystem |
| `ribbon*.h/cpp` | Ribbon toolbar UI (one tab per context) |
| `map*.h/cpp` | World/encounter map management |
| `monster*.h/cpp`, `bestiary*.h/cpp` | Monster/NPC data and Bestiary singleton |
| `character*.h/cpp`, `combatant*.h/cpp` | PC/NPC data model and UI widgets |
| `rule*.h/cpp` | Initiative and ruleset plugins |
| `overlay*.h/cpp` | Player-window overlays (fear meter, counters, timer) |
| `undo*.h/cpp` | Qt undo/redo stack entries (FoW painting, map markers, drawing) |
| `options*.h/cpp` | Settings container and QSettings accessor |
| `dmh*.h/cpp` | Cross-cutting utilities (logger, cache, VLC singleton, OpenGL debug) |

---

## 3. Build System

### Requirements
- **CMake ≥ 3.16**
- **Qt 6** (Core, Gui, Widgets, Xml, Multimedia, MultimediaWidgets, OpenGL, OpenGLWidgets, Network, UiTools)
- **libvlc** (headers + import library shipped in `vlc32/`, `vlc64/`, `vlcMac/`)
- MSVC 2022 (Windows) or AppleClang (macOS)

### Key CMakeLists.txt targets

```cmake
# Single executable target
add_executable(DMHelper
    ${DMHELPER_SOURCES}   # ~250 .cpp files
    ${DMHELPER_HEADERS}
    ${DMHELPER_FORMS}     # ~90 .ui files (CMAKE_AUTOUIC)
    ${DMHELPER_RESOURCES} # resources.qrc (CMAKE_AUTORCC)
)
```

All sources are listed **explicitly** (no globbing). `CMAKE_AUTOMOC ON` handles Q_OBJECT classes.

### Platform-specific linking

| Platform | VLC include path | Linked library |
|---|---|---|
| Windows 64-bit | `vlc64/` | `libvlc` (import lib) |
| Windows 32-bit | `vlc32/` | `libvlc` |
| macOS | `vlcMac/` | `vlc` |

The CMake `WIN32_EXECUTABLE TRUE` flag suppresses the Windows console window. On macOS `MACOSX_BUNDLE TRUE` packages into a `.app`.

### Building manually

```bash
# From DMHelper/src/
cmake -S . -B ../build-64_bit-release -DCMAKE_BUILD_TYPE=Release
cmake --build ../build-64_bit-release --config Release
```

### Full build + deploy scripts

- **Windows:** `DMHelper/buildanddeploy_msvc64_cmake.ps1` (PowerShell) — configures with CMake, runs `cmake --build`, calls `windeployqt`, copies VLC DLLs, creates installer with Qt IFW. Also callable via `buildanddeploy_msvc64_cmake.bat`.
- **macOS:** `DMHelper/buildanddeploy_macos_cmake.sh` (Bash) — same flow using `macdeployqt` and Qt IFW `binarycreator`.

Both scripts honour the `QT_ROOT_DIR` environment variable to override the default Qt installation path.

### Compile-time feature flags

| Macro | Effect |
|---|---|
| `INCLUDE_NETWORK_SUPPORT` | Enables `NetworkController` + DMHelperShared network classes (commented out by default in `dmconstants.h`) |
| `SHOW_UNSELECTED_SHAPE_SELECTION` | Shows selection handles on effect shapes |
| `NDEBUG` | Standard release flag; also disables `_DEBUG_NAME` member in `CampaignObjectBase` |
| `DMH_DEBUG_OPENGL 1` | Activates verbose per-call OpenGL logging via `DMH_DEBUG_OPENGL_Singleton` |
| `LAYERVIDEO_USE_OPENGL` | (commented out in `layervideo.h`) switches `LayerVideo` from CPU `VideoPlayer` to GPU `VideoPlayerGLPlayer` path |

---

## 4. Key Classes and Their Responsibilities

### Application Shell

| Class | File | Responsibility |
|---|---|---|
| `MainWindow` | `mainwindow.h/cpp` + `mainwindow.ui` | Top-level DM window. Owns the ribbon bar, campaign tree view, active frame stack, all dialog instances, `Campaign*`, `AudioPlayer*`, and the `PublishWindow*`. Drives the `activateObject()` / `deactivateObject()` cycle that swaps the central `CampaignObjectFrame`. |
| `PublishWindow` | `publishwindow.h/cpp` | The player-facing `QMainWindow`. Contains a single `PublishGLFrame*` (OpenGL widget). Receives a `PublishGLRenderer*` and delegates all GL calls to it. Emits mouse events back to the DM for pointer/interaction features. |
| `PublishGLFrame` | `publishglframe.h/cpp` | `QOpenGLWidget` subclass. Calls the active `PublishGLRenderer`'s `initializeGL()`, `resizeGL()`, `paintGL()`. Also hosts an `OverlayRenderer` for campaign overlays. |

### Data Model

| Class | File | Responsibility |
|---|---|---|
| `CampaignObjectBase` | `campaignobjectbase.h/cpp` | Abstract base for every serialisable campaign node. Provides UUID, name, icon, XML round-trip (`outputXML`/`inputXML`), `changed()`/`dirty()` signals, and a typed child-object list. Derives from `DMHObjectBase`. |
| `Campaign` | `campaign.h/cpp` | Root document object. Holds `Characterv2*`, `SoundboardGroup*`, `Overlay*` lists, date/time, fear counter, and a `Ruleset`. Serialises to/from an XML campaign file. |
| `BattleDialogModel` | `battledialogmodel.h/cpp` | Data model for one combat encounter. Owns the `LayerScene`, a priority-sorted `QList<BattleDialogModelCombatant*>`, a `QList<BattleDialogModelEffect*>`, map reference, camera rect, and a `BattleDialogLogger`. Persisted as child XML of `EncounterBattle`. |
| `LayerScene` | `layerscene.h/cpp` | Ordered container of `Layer*` objects. Provides both DM (QGraphicsScene-backed) and Player (OpenGL-backed) initialisation/paint cycles. Owned by `EncounterText`, `BattleDialogModel`, and `Map`. |
| `Layer` (abstract) | `layer.h/cpp` | Base for every rendering layer. Defines the dual-interface pattern: `dmInitialize/dmUninitialize/dmUpdate` for the DM's `QGraphicsScene` and `playerGLInitialize/playerGLUninitialize/playerGLPaint` for the OpenGL player view. Carries order, visibility (DM/player), opacity, position, and size. |

### Layer Concrete Types

| Class | File | Type enum |
|---|---|---|
| `LayerImage` | `layerimage.h/cpp` | `LayerType_Image` — static image tile |
| `LayerFow` | `layerfow.h/cpp` | `LayerType_Fow` — Fog-of-War mask, with `QUndoStack` |
| `LayerGrid` | `layergrid.h/cpp` | `LayerType_Grid` — configurable grid overlay |
| `LayerTokens` | `layertokens.h/cpp` | `LayerType_Tokens` — combatants and spell effects in battle |
| `LayerVideo` | `layervideo.h/cpp` | `LayerType_Video` — libvlc video playback layer |
| `LayerDraw` | `layerdraw.h/cpp` | `LayerType_Draw` — freehand drawing layer |
| `LayerEffect` | `layereffect.h/cpp` | `LayerType_Effect` — particle/animated effect layer |
| `LayerReference` | `layerreference.h/cpp` | `LayerType_Reference` — proxy pointing to another layer |

### Battle / Encounter

| Class | File | Responsibility |
|---|---|---|
| `BattleFrame` | `battleframe.h/cpp` + `battleframe.ui` | Central DM combat UI. Owns the `BattleDialogGraphicsScene`, the per-combatant widget list, the camera rect overlay, the `BattleFrameStateMachine`, and the active `PublishGLBattleRenderer*`. Coordinates initiative ordering, FoW painting, distance measurement, drawing engine, and all ribbon-triggered actions. |
| `BattleFrameStateMachine` | `battleframestatemachine.h/cpp` | Manages exclusive interaction modes (CombatantEdit, ZoomSelect, CameraSelect/Edit, Distance, FreeDistance, Pointer, FoWSelect/Edit, MapMove, Draw). Each state is a `BattleFrameState` with persistent vs. transient lifetime. |
| `BattleDialogGraphicsScene` | `battledialoggraphicsscene.h/cpp` | Subclass of `QGraphicsScene` with custom drag/drop, rubber-band selection, and token placement logic for the DM view. |

### OpenGL Renderer Hierarchy

```
PublishGLRenderer  (abstract base — initializeGL/resizeGL/paintGL + pointer painting)
├── PublishGLImageRenderer     — static image (text, scrolling, welcome screen)
├── PublishGLMapRenderer       — standalone map view
└── PublishGLBattleRenderer    (abstract — battle rendering + combatant tokens + effects)
    ├── PublishGLBattleImageRenderer   — image/FoW background battle
    └── PublishGLBattleVideoRenderer   — libvlc video background battle
```

`PublishGLBattleRenderer` manages GLSL shader programs (RGB / RGBA / RGBColor variants), projection matrix, token `QHash`, initiative bar, movement rings, active/selection token highlight overlays, and distance measurement line rendering.

### Video

| Class | File | Responsibility |
|---|---|---|
| `DMH_VLC` | `dmh_vlc.h/cpp` | Singleton. Initialises and owns the `libvlc_instance_t`. Manages a single `VideoPlayerGLVideo*` token pool via `requestVideo()`/`releaseVideo()`. |
| `VideoPlayer` | `videoplayer.h/cpp` | CPU-path video decoder. libvlc lock/unlock/display callbacks write decoded frames into a double-buffered `QImage`. Used by `LayerVideo` when `LAYERVIDEO_USE_OPENGL` is not defined. |
| `VideoPlayerGLPlayer` | `videoplayerglplayer.h/cpp` | GPU-path decoder. Drives libvlc to render directly into an `QOpenGLFramebufferObject` on an offscreen context, enabling zero-copy GPU upload. |
| `VideoPlayerGLVideo` | `videoplayerglvideo.h/cpp` | Manages a pool of up to 5 FBOs (`VIDEO_BUFFER_COUNT`). Implements the four libvlc OpenGL output callbacks: `setup`, `cleanup`, `swap`, `makeCurrent`, `getProcAddress`. |

### Audio

| Class | File | Responsibility |
|---|---|---|
| `AudioPlayer` | `audioplayer.h/cpp` | Thin wrapper around `QMediaPlayer`. Receives `AudioTrack*`, plays URLs, exposes volume/position/state signals. |
| `AudioTrack` (abstract) | `audiotrack.h/cpp` | Base for campaign audio entries. Concrete subclasses: `AudioTrackFile`, `AudioTrackUrl`, `AudioTrackYoutube`, `AudioTrackSyrinscape`, `AudioTrackSyrinscapeOnline`. |

### Settings and Infrastructure

| Class | File | Responsibility |
|---|---|---|
| `OptionsContainer` | `optionscontainer.h/cpp` | In-memory settings object with typed getters/setters and corresponding signals. Reads/writes via `OptionsAccessor` (a thin `QSettings` subclass). Drives `optionsdialog.ui`. |
| `DMHLogger` | `dmhlogger.h/cpp` | Installs a Qt message handler that writes timestamped logs to `QStandardPaths::AppDataLocation/logs/`. One instance per run. |
| `DMHCache` | `dmhcache.h/cpp` | Utility for building file paths inside the OS cache directory (token image cache, screenshot thumbnails, etc.). |
| `Bestiary` | `bestiary.h/cpp` | Singleton (`Bestiary::Instance()`). Maintains a `QMap<QString, MonsterClassv2*>` keyed by monster name. Reads/writes XML; implements `GlobalSearch_Interface`. |

---

## 5. Major Subsystems and How They Interact

### 5.1 Campaign Document

```
MainWindow
 └── Campaign (root CampaignObjectBase)
      ├── Party → Characterv2 list
      ├── Adventure → EncounterBattle / EncounterText / EncounterTextLinked
      │    └── EncounterBattle → BattleDialogModel → LayerScene → Layer[]
      ├── Map → LayerScene → Layer[]
      ├── AudioTrack list
      ├── SoundboardGroup list
      ├── Overlay list
      └── Ruleset
```

`CampaignObjectBase::outputXML()`/`inputXML()` drives XML serialisation for the entire tree. Every node emits `dirty()` when modified; `MainWindow` listens and marks the document as unsaved.

### 5.2 DM View — Frame Stack

`MainWindow::activateObject(CampaignObjectBase*)` selects the appropriate `CampaignObjectFrame` subclass:
- **`BattleFrame`** for `EncounterBattle`
- **`MapFrame`** for `Map`
- **`CharacterTemplateFrame`** for `Characterv2`
- **`EncounterTextEdit`** for text encounters

Each frame calls `deactivateObject()` on teardown. The `CampaignObjectFrameStack` manages the active stack so back-navigation works.

### 5.3 Player View — OpenGL Pipeline

```
MainWindow
 └── PublishWindow
      └── PublishGLFrame (QOpenGLWidget)
           └── PublishGLRenderer*  (swapped per active DM object)
                └── paintGL()
                     ├── paintBackground()   ← LayerImage / VLC FBO / solid color
                     ├── LayerScene::playerGLPaint()
                     │    └── each Layer::playerGLPaint()
                     ├── paintTokens / paintEffects
                     ├── paintInitiative bar
                     ├── paintPointer (mouse pointer image)
                     └── OverlayRenderer::paintGL()  ← overlays (fear, counter, timer)
```

The active renderer is set via `PublishWindow::setRenderer(PublishGLRenderer*)`. `BattleFrame::publishClicked()` creates a `PublishGLBattleImageRenderer` (or video variant) and emits `registerRenderer()` to `MainWindow`, which passes it to `PublishWindow`.

### 5.4 VLC Video Integration

`LayerVideo` (CPU path) creates a `VideoPlayer` and polls `isNewImage()` on a timer, compositing the decoded `QImage` onto its `QGraphicsPixmapItem`. The GPU path (`VideoPlayerGLPlayer`) renders via libvlc OpenGL output callbacks into an FBO on a dedicated offscreen `QOpenGLContext`; the FBO texture is then bound directly in `paintGL()`, avoiding any CPU readback.

`DMH_VLC::Initialize()` is called early in `MainWindow`'s constructor; `DMH_VLC::Shutdown()` in the destructor. The singleton's `requestVideo()/releaseVideo()` throttle concurrent VLC instances to avoid resource exhaustion.

### 5.5 Audio Subsystem

`AudioPlayer` (Qt Multimedia `QMediaPlayer`) handles local files and URLs. Syrinscape and YouTube `AudioTrack` subclasses dispatch to their respective external APIs. The `SoundboardFrame` hosts independent `SoundboardTrack` players for ambient mixing.

### 5.6 Battle State Machine

`BattleFrame` uses a `BattleFrameStateMachine` to manage mutually exclusive interaction modes (edit, zoom-select, camera-select, distance, FoW, draw, pointer). The machine is a simple list of `BattleFrameState` objects; activating one deactivates the previous. This avoids complex nested if/else logic throughout the frame's event handlers.

### 5.7 Undo/Redo

FoW painting and map marker operations are tracked through `QUndoStack` entries derived from `UndoFowBase` (`UndoFowFill`, `UndoFowPath`, `UndoFowPoint`, `UndoFowShape`) and `UndoMarker`. The undo stack is exposed by `MapFrame` and `LayerFow`.

---

## 6. Qt-Specific Patterns

### Signals and Slots

- **Ubiquitous** throughout the codebase. Every data-model class exposes `changed()` / `dirty()` signals for unsaved-state tracking.
- The pattern `connect(object, &Class::signal, this, &ThisClass::slot)` (new-style pointer-to-member) is used consistently.
- `Q_DECLARE_METATYPE(CampaignObjectBase*)` and similar declarations allow custom types to be passed through queued connections.

### .ui Files

There are roughly **90 `.ui` files**, all processed by `CMAKE_AUTOUIC`. Convention: a class `FooBar` has `foobar.h`, `foobar.cpp`, `foobar.ui`. The generated `Ui::FooBar` is always stored as a pointer `Ui::FooBar *ui` and accessed via `ui->widgetName`.

### Resource Files

`resources.qrc` bundles all images (icons, default tokens, cursors), GLSL shader source, and static data files. Access via `":/path/to/resource"` URLs throughout the code.

### Threading Model

- The main thread runs Qt's event loop.
- **libvlc** runs its own internal thread pool. The `VideoPlayer` CPU-path uses a `QMutex` (`_mutex`) to protect frame buffer access between the VLC decode thread and the Qt render/timer thread.
- **`VideoPlayerGLVideo`** introduces a semaphore (`_videoReady`) and a dedicated offscreen `QOpenGLContext`. libvlc's OpenGL callbacks (`makeCurrent`, `swap`) run on the VLC thread; the main GL thread reads the produced FBO.
- `main.cpp` sets `Qt::AA_DontCheckOpenGLContextThreadAffinity` specifically to allow `makeCurrent()` calls from the VLC thread.
- `ObjectImportWorker` (`objectimportworker.h/cpp`) and `ExportWorker` (`exportworker.h/cpp`) run on `QThread` for long-running import/export operations.

### OpenGL Usage

All GL rendering goes through `QOpenGLWidget` (`PublishGLFrame`). Shaders are inline strings (or loaded from `resources.qrc`). The helper macros in `dmh_opengl.h` (e.g. `DMH_DEBUG_OPENGL_glUseProgram(x)`) wrap every GL call with debug logging when `DMH_DEBUG_OPENGL=1`; they compile away to nothing in release builds.

Three GLSL programs are used in the battle renderer:
- `shaderProgramRGB` — opaque textured quads
- `shaderProgramRGBA` — transparent textured quads (alpha-controlled)
- `shaderProgramRGBColor` — flat-colored quads (movement rings, distance line)

---

## 7. Third-Party Dependency: libvlc

**What it is:** libvlc is the embeddable C API of VLC media player. It provides hardware-accelerated decoding of virtually any video/audio format.

**Why it is used:** Qt Multimedia alone does not guarantee hardware-accelerated video decoding across platforms, and the project needs to play video files as live animated map backgrounds or layer overlays in real-time at high frame rates during gameplay.

**How it is integrated:**

| Integration point | File | Detail |
|---|---|---|
| Singleton init/shutdown | `dmh_vlc.cpp` | `libvlc_new()` / `libvlc_release()` |
| CPU decode path | `videoplayer.cpp` | `libvlc_video_set_callbacks()` (lock/unlock/display) + `libvlc_video_set_format_callbacks()` |
| GPU decode path | `videoplayerglvideo.cpp` | `libvlc_video_set_output_callbacks()` with OpenGL type — renders into FBOs |
| VLC event handling | `videoplayer.cpp`, `videoplayerglplayer.cpp` | `libvlc_event_attach()` → static callback → `QMetaObject::invokeMethod` to marshal to main thread |

The VLC binaries (DLLs on Windows, dylib on macOS) are **not built from source**. They are pre-built and shipped in `vlc32/`, `vlc64/`, `vlcMac/` inside the repository and copied to the deploy directory by the build scripts.

---

## 8. Coding Conventions

### Naming

| Element | Convention | Example |
|---|---|---|
| Classes | `UpperCamelCase` | `BattleDialogModel`, `LayerFow` |
| Methods / slots | `lowerCamelCase` | `getCameraRect()`, `setCameraRect()` |
| Signals | `lowerCamelCase`, descriptive past or noun | `cameraRectChanged()`, `layerAdded()` |
| Member variables | `_lowerCamelCase` (single underscore prefix) | `_campaign`, `_targetSize` |
| Local variables | `lowerCamelCase` | `newRenderer` |
| Enums | `TypeName_ValueName` | `CampaignType_Battle`, `LayerType_Fow` |
| Constants | `SCREAMING_SNAKE_CASE` | `STARTING_GRID_SCALE`, `CURSOR_SIZE` |
| Namespaces | `UpperCamelCase` | `DMHelper::` (sole namespace for enums/constants) |

### File Organisation

- **One class per `.h`/`.cpp` pair**, named identically in lowercase (e.g. `BattleDialogModel` → `battledialogmodel.h/.cpp`).
- `.ui` files co-located with their class in `src/`.
- All files live flat in `src/` — no subdirectory grouping (except `data/`, `bestiary/`, `vlc*/`).

### Patterns to Follow

1. **XML serialisation**: Override `createOutputXML()` (returns the element tag) and `internalOutputXML()` (writes attributes/children). Call `super` via `CampaignObjectBase::internalOutputXML()`. For reading, override `inputXML()` and call `postProcessXML()` for cross-reference resolution after the full tree is loaded.
2. **Dual DM/Player interface**: Every `Layer` subclass must implement both `dmInitialize()/dmUninitialize()/dmUpdate()` (QGraphicsScene path) and `playerGLInitialize()/playerGLUninitialize()/playerGLPaint()` (OpenGL path). Neither path should assume the other is active.
3. **Dirty tracking**: Emit `dirty()` for unsaved data changes, `changed()` for visual-only updates. Do not call `emit dirty()` inside constructors or `inputXML()`.
4. **Singletons**: `Bestiary::Instance()`, `DMH_VLC::DMH_VLCInstance()`. Initialised with static `Initialize()` / torn down with `Shutdown()`. Never construct directly.
5. **Renderer lifecycle**: `PublishGLRenderer` subclasses must implement `initializeGL()` / `cleanupGL()` symmetrically. Call `cleanupGL()` before deletion. `deleteOnDeactivation()` returns `true` for renderers that are created per-publish and should be deleted when the object is deactivated.

---

## 9. Known Tricky Areas and Technical Debt

### VLC Thread Safety
The libvlc decode callbacks (`lockCallback`, `unlockCallback`, `displayCallback`) in `VideoPlayer` run on **VLC's internal thread**, not the Qt main thread. The `QMutex _mutex` in `VideoPlayer` protects the double buffer. Any access to Qt GUI objects from these callbacks must use `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`. Violating this causes crashes that are hard to reproduce.

The GPU path (`VideoPlayerGLVideo`) adds further complexity: `makeCurrent` and `swap` run on the VLC thread, which means `QOpenGLContext::makeCurrent()` crosses thread boundaries — this is why `Qt::AA_DontCheckOpenGLContextThreadAffinity` is set in `main.cpp`.

### Single VLC Instance Pool
`DMH_VLC` only manages one active `VideoPlayerGLVideo` at a time (`_currentVideo`). Multiple simultaneous GPU video layers are not fully supported; a `requestVideo()`/`releaseVideo()` + timer-based retry mechanism is in place but is a known limitation.

### `INCLUDE_NETWORK_SUPPORT` is Disabled
The `NetworkController` class and the entire `DMHelperShared` network stack are compiled in but gated by `#ifdef INCLUDE_NETWORK_SUPPORT` (commented out in `dmconstants.h`). The shared source files are still listed in `CMakeLists.txt` and compiled unconditionally, but the network features are not wired up. This dead-code path may drift out of sync.

### Duplicate UUID Detection
`Campaign::validateCampaignIds()` / `correctDuplicateIds()` are called on load to detect and optionally repair duplicate `QUuid` values that can occur from copy-paste or import operations. This is a known fragility of the XML-based data model.

### `LayerVideo` CPU vs GPU Toggle
The compile-time `#define LAYERVIDEO_USE_OPENGL` in `layervideo.h` is commented out, meaning video layers default to the CPU decode path. The GPU path code exists but is not exercised in normal builds.

### `monsterclass.h` vs `monsterclassv2.h`
There are two parallel monster class representations. `MonsterClass` is the older format; `MonsterClassv2` (with `MonsterClassv2Converter`) is the current one used by `Bestiary`. The old `MonsterClass` remains for legacy file compatibility. New code should use `MonsterClassv2` exclusively.

### `character.h` vs `characterv2.h`
Similarly, `Character` is a legacy class; `Characterv2` (with `CharacterV2Converter`) is the current model. `Characterv2` inherits from both `Combatant` and `TemplateObject` (the latter enabling data-driven UI via `.ui` files loaded at runtime with `Qt::UiTools`).

### No Automated Tests
`DMHelperTest/` is archived and empty. There is no CI test suite. All validation is manual. New features should be written with testability in mind (pure functions, injectable dependencies) even though tests cannot currently be run automatically.

### Flat Source Directory
All ~250 `.cpp` files live in a single `src/` directory. Navigation relies entirely on consistent naming conventions. Any refactoring to subdirectories would require updating all `#include` paths and the full explicit source list in `CMakeLists.txt`.
