# Repository Guidelines

## Project Structure & Module Organization

Ngin is a C++23 rendering engine skeleton. DirectX 12 is first; Vulkan is planned through the same RHI contracts. Public headers live in `src/ngin/include/ngin/`, implementations in `src/ngin/src/`, and the sample app in `src/sandbox/`. Docs are in `docs/`, dependencies in `vendor/`, generated projects in `build/`, and outputs in `bin/` and `bin-int/`.

Module roles: `core` for shared types, assertions, diagnostics, and utilities; `platform` for windows, input, timing, and OS services; `renderer` for engine APIs; `rhi` for backend contracts; `rhi_dx12` for DX12; `dev_ui` for ImGui debug tooling.

## Architecture Rules

Dependencies flow from app/tools to `renderer`, then `rhi`, then a backend such as `rhi_dx12`. Shared code must not include DX12 or Vulkan headers directly. Native graphics objects/handles, including devices, queues, swapchains, command lists, descriptors, fences, root signatures, and pipelines, belong in backend modules. Shared 2D/3D systems use backend-neutral descriptions for resources, materials, shaders, frame scheduling, mesh/sprite submission, and render passes. ImGui panels live in `dev_ui`; backend binding lives beside the backend.

## Build, Test, and Development Commands

Windows:

```powershell
./build.ps1 gen [gmake|vs2022|vs2026]
./build.ps1 build [DEBUG|STAGING|RELEASE]
./build.ps1 run [DEBUG|STAGING|RELEASE]
./build.ps1 clean
```

`gen` runs Premake5. `build` compiles the selected config and defaults to `DEBUG`. `run` builds and launches the sandbox executable. `clean` removes generated files and outputs only. `build.sh` is future-facing; Windows/DX12 is current. Clang is primary; keep MSVC 2022 compatibility.

Use uppercase configs: `DEBUG` enables diagnostics/dev UI, `STAGING` keeps diagnostics/dev UI with release-like optimization, and `RELEASE` excludes ImGui and developer-only UI.

## Coding Style & Naming Conventions

Format C++ with `.clang-format`: Google style, 140-column limit. Use lowercase folder names and underscores in paths. Use the `Ngin` namespace and include public engine headers as `<ngin/...>`. Guard dev UI with `#if NGIN_DEV_UI`.

## Collaboration Rules

Do not correct code, edit files, or show code snippets unless the user explicitly asks for that. When troubleshooting, explain the cause and guidance in plain language first.

When a tracked next step or TODO in this file is completed, remove it from the needed-next-step list or move it into current status during the same review.

## Testing Guidelines

No test framework yet. Keep the console-based vertical slice working:

```powershell
./build.ps1 gen
./build.ps1 run DEBUG
./build.ps1 run STAGING
```

Run `./build.ps1 run RELEASE` when touching dev UI guards or config logic. Current status: Win32 window creation and explicit RHI lifetime/cleanup ownership are in place, including owned DX12 backbuffers and root signature storage. Needed next steps: finish DX12 RHI object creation, then implement the renderer layer that owns initialization, frame rendering, resize handling, and shutdown. Next milestones: minimal DX12 clear-color rendering, then hide private engine/backend details from programs that use Ngin, then ImGui for `DEBUG` and `STAGING`.

## Commit & Pull Request Guidelines

Use short conventional-style subjects like `docs: add struct docs`. Keep commits concise, imperative, and scoped. PRs should describe changes, validation commands, linked issues, and renderer-visible captures.

## Security & Configuration Tips

Do not commit generated directories, binaries, logs, or local tool artifacts. Keep dependencies under `vendor/`. `RELEASE` must stay free of debug panels, developer menus, and staging diagnostics.

## Update

Update this agents.md file everytime you notice the project progressing
