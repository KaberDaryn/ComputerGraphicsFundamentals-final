# Dream Developer Desk

An interactive C++17 / OpenGL 3.3 Core developer studio for the Computer Graphics
Fundamentals final project. The room includes a dual-monitor desk, RGB keyboard,
PC, sofa, bookcase, lamps, Kazakhstan flag and woven rug. A second camera renders
a live view onto the right monitor. The room camera supports an isometric view.

## Build and run

Requires CMake 3.21+, a C++17 compiler and an OpenGL 3.3-capable graphics driver.
Third-party source dependencies are bundled in `external`; textures and shaders
are bundled in `assets`. No downloads or asset regeneration are required at runtime.

Windows with MSYS2 UCRT64 GCC/Make and CMake installed, from the project root:

```powershell
$env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
cmake -S . -B build -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 8
./build/DreamDeveloperDesk.exe
```

Windows with Visual Studio 2022 and the Desktop development with C++ workload:

```powershell
cmake -S . -B build -G 'Visual Studio 17 2022' -A x64
cmake --build build --config Release
./build/Release/DreamDeveloperDesk.exe
```

Linux needs a C++ toolchain, CMake, OpenGL and the X11 development headers used by
GLFW. Configure with `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`, build with
`cmake --build build --parallel`, then run `./build/DreamDeveloperDesk`.
The delivered source has been verified on Windows GCC; MSVC/Linux are not certified.

CMake copies `assets` beside the executable. Retain that directory when moving
the built program. A MinGW build may also require the GCC runtime DLLs beside
the executable or in PATH. This archive contains source, not a prebuilt EXE.

## Controls

| Input | Action |
| --- | --- |
| Left drag / WASD | Orbit camera |
| Wheel / Q / E | Zoom |
| 1 / 2 / 3 / 4 | Room / desk / lounge / top view |
| L | Toggle desk lamp |
| T / Y | Reading lamp / shelf lighting |
| B | Open/close cabinet drawers |
| C | Change the left monitor image |
| M | Change monitor layout |
| R | Cycle RGB mode |
| [ / ] | Adjust RGB speed |
| N | Day/night |
| P | Toggle live camera monitor |
| F | Start/restore RGB finale |
| H | Hide/show controls |
| Esc | Exit |

The panel also switches parallel projection, filtered daylight shadows and light
glow. The room has a shared aligned wall/floor contour. Reading/shelf lighting
controls lock during the finale; camera and drawer controls remain available.
Clocks and editor/terminal monitor graphics are decorative displays.

## Tests and optional C++ tools

```powershell
cmake -S . -B build -DDREAM_DESK_BUILD_TOOLS=ON
cmake --build build --parallel 8
ctest --test-dir build --output-on-failure
```

With the tools enabled, three CPU tests cover the finale, camera/projection and
PNG round trips. The optional generators use only C++17 and the standard library:

```powershell
./build/generate_rug.exe ./build/generated-textures
./build/generate_studio_assets.exe ./build/generated-textures
```

For Visual Studio, use `build/Release` and pass `-C Release` to CTest. The bundled
textures already work; generated images use the same design family and may differ
from those assets. All project-owned programs are C++17, shaders use GLSL, and
vendored GLFW/GLAD retain their original C source. There are no Python files.

## Rendering and diagnostics

Rendering uses indexed meshes, a parent/child scene graph, Blinn-Phong materials,
six lights, cached 2048x2048 daylight shadows with 3x3 PCF, a transparent PC pane
and a full-window edge-smoothing/bright-pass glow stage. Local lamps contribute
lighting without separate shadow maps; the glow is LDR, not an HDR/PBR pipeline.

Set `DDD_CAPTURE` to `day`, `night`, `desk`, `lounge`, `top`, `side`, `drawers`,
`terminal`, `wallpaper`, `lights-off`, `effects-off`, `resize`, `angled`, `wide`,
`feed-off`, `lamp-off`, `finale`, `restore` or `panel` to export a diagnostic PPM
frame and exit. `resize` checks a running-window resize to 1440x900.
Set `DDD_BENCHMARK` to `feed`, `off` or `finale` for a fixed-camera, VSync-off run
with 2 s warmup and 15 s sampling. Do not combine benchmark and capture modes.
Normal interactive runs retain VSync. CSV benchmarks and captures are local output.

## Responsibility allocation and credits

Each participant is assigned 702 own-source C++/GLSL lines, excluding blank lines,
comments and third-party libraries. Shared assets/build files do not affect this count.

| Participant | Code responsibility | Code lines |
| --- | --- | ---: |
| Kaber Daryn | Application, main, Camera, CPU-тесты, shadow GLSL | 702 |
| Arthur Kopzhasarov | Mesh, Primitives, Shader, Framebuffer, ShadowMap, Lighting, PNG-helper, post/sky GLSL | 702 |
| Ernar Sadenov | Room/Desk/Studio/SceneFactory, RoomLayout, C++-генераторы, basic GLSL | 702 |
| Erkebulan Sovetov | Scene, SceneNode, WowSequence, Texture, UiPanel, RGB/screen GLSL | 702 |

OpenAI ChatGPT/Codex assisted with architecture, C++/GLSL implementation,
procedural textures, build verification, tests and documentation. Team members
should review and understand their modules and accurately describe their own
work. This allocation does not replace the actual commit history.

See [CREDITS.md](CREDITS.md) and the bundled third-party license notices for asset
sources and dependency attribution.
