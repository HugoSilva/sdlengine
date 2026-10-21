# RSE (RockSlide Engine) Alpha 0.13

Is a small C++ game engine that is heavily reliant on SDL3, it is work in progress with not many features available, the main objective is to gain knowledge on what it takes to create a stable 2D game engine following the KISS principle.

## Build target

The project is self-sustained: every dependency lives in the `Thirdparty/` folder. It currently targets **Windows** (x86 / x64) and is built with **CMake + Ninja + MSVC (C++20)**.

## Building

All build configuration lives in CMake. There is a single entry point:

```sh
cmake --preset x64-debug
cmake --build --preset x64-debug
```

Other presets: `x64-release`, `x86-debug`, `x86-release`.

> The Ninja generator targets whatever toolchain your shell provides. Run `x64-*`
> from the **x64** developer prompt and `x86-*` from the **x86** developer prompt
> (`vcvars64.bat` / `vcvars32.bat`).

### Prerequisites

- Visual Studio 2022 (or Build Tools) with the C++ workload — provides `cl.exe`, `cmake` and `ninja`.
- Run the commands from a **Developer Command Prompt** (so `cl.exe` / `ninja` are on `PATH`),
  or use the CMake/Ninja bundled with Visual Studio:
  `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\`.

### Targets

| Target      | Kind            | Notes                                            |
|-------------|-----------------|--------------------------------------------------|
| `Engine`    | static library  | The engine itself.                               |
| `Editor`    | executable      | ImGui based scene/entity editor.                 |
| `Game`      | executable      | Sample project.                                  |
| `UnitTests` | executable      | GoogleTest tests (built from source via FetchContent).|

Each executable gets its runtime DLLs, its `Resources/` folder and the `SampleProject/` directory copied next to it automatically.

Toggle targets off with `-DRSE_BUILD_EDITOR=OFF`, `-DRSE_BUILD_GAME=OFF`, `-DRSE_BUILD_TESTS=OFF`.

### Projects

A game is a project: a directory with a `project.xml` manifest (window, asset root, startup scene, scene list) and one file per scene. Both binaries take the manifest's path as their first argument and fall back to `./project.xml`:

```bat
cd bin\x64-debug
Game.exe SampleProject\project.xml
```

`SampleProject/` is checked in and deployed next to the binaries, so the command above works out of the box. [`docs/ProjectFormat.md`](docs/ProjectFormat.md) documents the manifest, the scene files, and what they do and do not store.

The editor keeps its own state in `editor.cfg` in the working directory, next to `imgui.ini`: the grid, the gizmo, which panels are visible, the last scene, and the five most recent projects. Run it without a path and it reopens the most recent one; `File > Open Recent` lists the rest. A project that has since been moved is dropped from the list instead of failing, and with none left the editor falls back to `./project.xml`, so a renamed folder cannot stop the editor from starting.

`Edit > Project Settings` sets the project's master, music and sound-effect levels. They are stored in the manifest, so the game opens at the same mix the editor was tuned with.

`Scenes` lists the manifest's scenes with a tick on the open one; `Ctrl+Tab` and `Ctrl+Shift+Tab` walk the list, wrapping at the ends. Switching, and `New Scene...`, ask before unsaved edits are discarded. The content browser is rooted at the project directory and opens in `scenes/`: double-click a scene there to open it, or right-click for open, rename, delete and `Show in Explorer`.

Keys are rebindable from the manifest too: the `input` section maps actions such as `editor.save` and `game.up` to keys like `Ctrl+S` or `Space`. A project that names an action this build does not know gets a warning and keeps the default — see [the manifest's input bindings](docs/ProjectFormat.md#input-bindings).

### Tests

```sh
ctest --preset x64-debug
```

GoogleTest is built from source (fetched automatically via CMake `FetchContent`, pinned to
`v1.18.0`), so `UnitTests` works for every architecture and configuration. For offline builds
point `-DRSE_GTEST_SOURCE_DIR=<path>` at a local GoogleTest checkout, or disable tests with
`-DRSE_BUILD_TESTS=OFF`.

## Coding standards

The project follows the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with a
short list of deliberate deviations. [`docs/CodingStandards.md`](docs/CodingStandards.md) has the rules and
the reasons; [`.clang-format`](.clang-format) and [`.clang-tidy`](.clang-tidy) are the machine-readable half
of it, and they are what CI enforces.

```bat
tools\format.bat                 :: reformat every source file in place
tools\check-format.bat           :: fail if any file is not clang-format clean
tools\check-naming.bat x64-debug :: naming check (needs: cmake --preset x64-debug)
```

## Library dependencies

##### [Glew 2.3.1](http://glew.sourceforge.net/) - OpenGL extensions and methods
##### [SDL 3.4.18](https://www.libsdl.org/) - Creating the window
##### [SDL_image 3.4.8](https://github.com/libsdl-org/SDL_image) - Reading images
##### [SDL_mixer 3.2.4](https://github.com/libsdl-org/SDL_mixer) - Loading sounds
##### [SDL_ttf 3.2.2](https://github.com/libsdl-org/SDL_ttf) - Rendering fonts
##### [GLM 1.0.3](https://github.com/g-truc/glm) - Math library
##### [IMGUI 1.93.0](https://github.com/ocornut/imgui) - GUI framework
##### [Box2D 2.4.1](https://github.com/erincatto/box2d) - 2D Physics engine
##### [cereal 1.3.2](https://github.com/USCiLab/cereal) - Serialization
##### [EnTT 2.x](https://github.com/skypjack/entt) - Entity component system
##### [rapidxml 1.13](http://rapidxml.sourceforge.net/) - XML parsing
##### [GoogleTest 1.18.0](https://github.com/google/googletest) - Unit testing (via CMake FetchContent)
