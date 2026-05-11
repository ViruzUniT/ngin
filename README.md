# Ngin Renderer

Ngin is a C++23 rendering engine project focused on high-performance 3D and 2D rendering. The first real graphics backend is DirectX 12 on Windows, but the engine is structured so most code can stay shared when a Vulkan backend is added later.

The renderer should be modular from the start. Application code should talk to engine and renderer interfaces, not directly to DirectX 12 objects. Backend-specific code belongs behind the rendering hardware interface.

## Goals

- Build a modern C++23 renderer with both 3D and 2D rendering support.
- Use DirectX 12 as the first production backend.
- Keep the renderer backend-agnostic enough to support Vulkan later.
- Use Clang as the primary compiler.
- Maintain MSVC compatibility for Windows and DirectX 12 development.
- Use ImGui as a development and staging debug/testing UI.
- Exclude ImGui and debug UI code from final release builds.
- Use Premake5 to generate project files.
- Provide wrapper scripts for common build, run, generate, and clean commands.

## Architecture

The engine should be split into layers with clear ownership:

| Layer | Responsibility |
| --- | --- |
| `Core` | Logging, assertions, common types, diagnostics, memory helpers, platform-neutral utilities. |
| `Platform` | Windows, input, timing, file dialogs, OS integration, and future Linux platform support. |
| `Renderer` | Public rendering API used by the engine, app, tools, and 2D/3D systems. |
| `RHI` | Backend abstraction for graphics devices, swapchains, queues, command lists, buffers, textures, descriptors, pipelines, and synchronization. |
| `RHI_DX12` | DirectX 12 implementation of the RHI. This is the first real backend. |
| `RHI_Vulkan` | Future Vulkan implementation of the same RHI contracts. |
| `DevUI` | ImGui debug and testing UI for development and staging builds only. |

More detail: [Architecture](docs/architecture.md).

## Rendering Backend Strategy

DirectX 12 is the first target because it is the best fit for the initial Windows renderer. The long-term design should still avoid locking the entire engine to DX12.

Backend-specific implementation details must stay inside backend modules:

- Native DX12 device, command queue, descriptor heap, command allocator, command list, fence, swapchain, and pipeline objects stay in `RHI_DX12`.
- Shared renderer systems should use RHI interfaces and renderer-owned abstractions.
- Shader, pipeline, and resource descriptions should be backend-neutral where practical.
- Vulkan support should be added by implementing the same RHI behavior in `RHI_Vulkan`, not by rewriting renderer-facing systems.

More detail: [Backend Policy](docs/backend-policy.md).

## Build Configurations

The project uses uppercase build configuration names.

| Config | Purpose | Optimization | Debug Extras | ImGui |
| --- | --- | --- | --- | --- |
| `DEBUG` | Development and debugging. | Off or minimal. | Enabled. | Enabled. |
| `STAGING` | Release-like testing with diagnostics. | Release-like. | Enabled. | Enabled. |
| `RELEASE` | Final user-facing build. | Enabled. | Disabled. | Disabled. |

`STAGING` exists to catch issues that only appear with compiler optimizations, asset packaging, or release-like runtime behavior while still keeping diagnostics and the ImGui testing UI available.

`RELEASE` must not compile in ImGui, debug menus, test panels, or developer-only renderer controls.

## ImGui Policy

ImGui is used as a debug and testing UI, not as the final application UI.

Rules:

- ImGui is available in `DEBUG`.
- ImGui is available in `STAGING`.
- ImGui is not available in `RELEASE`.
- Direct ImGui usage in engine or app code must be guarded by dev/staging build macros.
- Backend-specific ImGui setup lives beside the graphics backend that owns it.
- DX12 ImGui integration is implemented first.
- Vulkan ImGui integration is added later when the Vulkan backend exists.

Expected compile-time control:

```cpp
#if NGIN_DEV_UI
// ImGui debug/testing panels.
#endif
```

`NGIN_DEV_UI` should be enabled for `DEBUG` and `STAGING`, and disabled for `RELEASE`.

## Toolchain

Primary compiler:

- Clang

Compatibility compiler:

- MSVC 2022

Required Windows components:

- Windows 10 or Windows 11
- Windows SDK with DirectX 12 headers and libraries
- A build environment capable of compiling DX12 code
- Premake5 available on `PATH`

Linux support is structural at first. The project may provide scripts and source layout that make Linux support straightforward later, but the first real graphics backend is DX12 and therefore Windows-only.

## Premake5 Workflow

Premake5 is responsible for generating IDE/build files.

Generated files should be placed under:

```txt
build/
```

Compiled outputs should be placed under:

```txt
bin/
bin-int/
```

The first supported Premake action should target Visual Studio for DX12 development. Future Vulkan/Linux work can add other Premake actions when that backend is implemented.

More detail: [Build Workflow](docs/build.md).

## Build Wrapper

The project should provide two wrapper scripts:

- `build.ps1` for Windows PowerShell.
- `build.sh` for Linux/macOS-style shells and future cross-platform workflows.

Both wrappers should expose the same command shape where possible.

```sh
./build.ps1 gen
./build.ps1 build [DEBUG|STAGING|RELEASE]
./build.ps1 run [DEBUG|STAGING|RELEASE]
./build.ps1 clean
```

```sh
./build.sh gen
./build.sh build [DEBUG|STAGING|RELEASE]
./build.sh run [DEBUG|STAGING|RELEASE]
./build.sh clean
```

If no configuration is passed to `build` or `run`, the wrapper must default to:

```txt
DEBUG
```

Examples:

```sh
./build.ps1 build
./build.ps1 run
./build.ps1 build STAGING
./build.ps1 run RELEASE
./build.ps1 clean
```

`build.sh` exists to keep the workflow portable, but DX12 build and run behavior is Windows-only until a Vulkan backend exists.

## Dependencies

Third-party dependencies should be managed through Git submodules once this project is inside a git repository.

Initial dependency:

- Dear ImGui

Important: Git submodules require a `.git` directory. If this project has not been initialized as a git repository yet, initialize git before adding submodules.

Expected dependency layout:

```txt
vendor/
  imgui/
```

Expected setup flow after git initialization:

```sh
git submodule add https://github.com/ocornut/imgui vendor/imgui
git submodule update --init --recursive
```

## Current Limitations

- DX12 is the only planned initial backend.
- Vulkan is planned but not implemented initially.
- Linux rendering is planned through Vulkan, not DX12.
- ImGui is for debug/testing UI only.
- The release build must remain free of dev UI code.

## Roadmap

1. Create the Premake5 project layout.
2. Add the build wrapper scripts.
3. Add the core engine and platform skeleton.
4. Add the RHI interfaces.
5. Implement the DX12 backend.
6. Add ImGui support for `DEBUG` and `STAGING`.
7. Add simple 2D and 3D renderer paths.
8. Add Vulkan backend support later using the same RHI contracts.
