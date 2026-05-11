# Architecture

Ngin should separate engine-facing renderer code from backend-specific graphics API code. The main purpose of the architecture is to keep DirectX 12 implementation details from spreading through the whole project, so Vulkan can be added later without rewriting the renderer.

## Layers

`Core` contains shared infrastructure: logging, assertions, common types, diagnostics, memory helpers, and platform-neutral utilities. It must not depend on a graphics backend.

`Platform` owns OS-facing behavior such as windows, input, timing, and platform services. Windows support is required first for DX12. Linux support is future-facing and should not force the initial DX12 implementation to become cross-platform before it exists.

`Renderer` is the engine-facing rendering layer. It exposes the APIs that the app, tools, 2D renderer, and 3D renderer use. It should describe rendering work in backend-neutral terms.

`RHI` is the rendering hardware interface. It defines the common contracts for devices, queues, swapchains, command lists, buffers, textures, descriptors, pipelines, and synchronization.

`RHI_DX12` implements the RHI using DirectX 12. It owns native DX12 objects and DX12-specific lifetime/synchronization details.

`RHI_Vulkan` will implement the same RHI contracts using Vulkan later. It should not require app or high-level renderer code to be rewritten.

`DevUI` owns ImGui debug/testing panels and backend-specific ImGui setup. It is compiled only when dev UI is enabled.

## Dependency Direction

Dependencies should flow inward:

```txt
App / Tools
  -> Renderer
    -> RHI
      -> RHI_DX12 or RHI_Vulkan
```

High-level code must not include DX12 or Vulkan headers directly. Backend headers should stay in backend modules.

## 3D Rendering

The 3D renderer should use shared renderer and RHI systems for:

- Mesh buffers.
- Material data.
- Texture resources.
- Pipeline descriptions.
- Camera and transform data.
- Render passes or equivalent frame graph concepts.

Backend-specific code should translate those descriptions into native DX12 objects first, and Vulkan objects later.

## 2D Rendering

The 2D renderer should also use the same renderer/RHI path. It should not be a separate graphics backend.

Expected 2D systems include:

- Batched textured quads.
- Sprite atlases.
- SDF or atlas-based text.
- Simple debug drawing.
- UI/testing overlays for development builds.

## Dev UI

ImGui belongs in the `DevUI` layer and backend integration code. It should not become a required dependency of release runtime systems.

Use a build macro such as:

```cpp
#if NGIN_DEV_UI
// ImGui-only code.
#endif
```

`NGIN_DEV_UI` is enabled only for `DEBUG` and `STAGING`.
