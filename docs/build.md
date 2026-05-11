# Build Workflow

The project uses Premake5 for project generation and wrapper scripts for common commands.

## Build Configurations

Use uppercase configuration names:

```txt
DEBUG
STAGING
RELEASE
```

`DEBUG` is the default configuration for `build` and `run` when no configuration argument is passed.

`STAGING` should use release-like optimization while keeping assertions, diagnostics, validation, and ImGui dev UI enabled.

`RELEASE` should be optimized and must not compile ImGui or developer-only UI code.

## Toolchains

Clang is the primary compiler.

MSVC 2022 compatibility should be maintained for Windows and DirectX 12 development. The project should not rely on compiler-specific behavior unless it is isolated and documented.

## Premake

Premake5 should generate project files into:

```txt
build/
```

Output directories should be:

```txt
bin/
bin-int/
```

The first supported generated target is a Visual Studio solution for DX12 development on Windows.

## Wrapper Commands

Windows:

```sh
./build.ps1 gen
./build.ps1 build [DEBUG|STAGING|RELEASE]
./build.ps1 run [DEBUG|STAGING|RELEASE]
./build.ps1 clean
```

Linux/macOS-style shell:

```sh
./build.sh gen
./build.sh build [DEBUG|STAGING|RELEASE]
./build.sh run [DEBUG|STAGING|RELEASE]
./build.sh clean
```

Default behavior:

- `build` without a config builds `DEBUG`.
- `run` without a config builds and runs `DEBUG`.
- `clean` removes generated project files, binaries, and intermediate output.
- `gen` runs Premake generation.

`build.sh` is part of the planned cross-platform workflow, but DX12 build and run support is Windows-only until Vulkan exists.

## Expected Clean Targets

The clean command should remove generated and compiled files:

```txt
build/
bin/
bin-int/
```

It should not remove source code, dependencies, documentation, or user assets.
