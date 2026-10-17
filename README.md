# RSE (RockSlide Engine) Alpha 0.13

Is a small C++ game engine that is heavily reliant on SDL2, it is work in progress with not many features available, the main objective is to gain knowledge on what it takes to create a stable 2D game engine following the KISS principle.

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
| `UnitTests` | executable      | GoogleTest tests (skipped when gtest is missing).|

Each executable gets its runtime DLLs and its `Resources/` folder copied next to it automatically.

Toggle targets off with `-DRSE_BUILD_EDITOR=OFF`, `-DRSE_BUILD_GAME=OFF`, `-DRSE_BUILD_TESTS=OFF`.

### Tests

```sh
ctest --preset x64-debug
```

> Note: the vendored GoogleTest is currently prebuilt for **Win32/Debug** only, so `UnitTests`
> is skipped for other architectures/configurations.

## Library dependencies

##### [Glew 2.1.0](http://glew.sourceforge.net/) - OpenGL extensions and methods
##### [SDL 2.0.7](https://www.libsdl.org/) - Creating the window
##### [SDL_image 2.0.2](https://www.libsdl.org/projects/SDL_image/) - Reading images
##### [SDL_mixer 2.0.2](https://www.libsdl.org/projects/SDL_mixer/) - Loading sounds
##### [SDL_ttf 2.0.14](https://www.libsdl.org/projects/SDL_ttf/) - Rendering fonts
##### [GLM 0.9.9-a2](https://glm.g-truc.net/) - Math library
##### [IMGUI 1.60](https://github.com/ocornut/imgui) - GUI framework
##### [Box2D 2.3.1](http://box2d.org/) - 2D Physics engine
##### [Assimp 3.3.1](http://assimp.sourceforge.net/) - Loading 3D models
