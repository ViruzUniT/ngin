# Backend Policy

The renderer starts with DirectX 12, but the engine should be written so Vulkan can be added later with minimal disruption to high-level systems.

## Backend Isolation

DX12-specific types must stay inside the DX12 backend. That includes:

- Devices.
- Command queues.
- Command allocators and command lists.
- Descriptor heaps.
- Swapchains.
- Fences.
- Root signatures.
- Pipeline state objects.
- Native resource handles.

High-level renderer code should use backend-neutral descriptions and RHI handles instead of native graphics API objects.

## Shared Renderer Code

The following systems should be shared by DX12 and future Vulkan backends:

- Renderer-facing resource descriptions.
- Material and shader metadata.
- Mesh and sprite submission.
- Frame scheduling concepts.
- 2D batching.
- 3D scene submission.
- Dev UI panel definitions, excluding backend-specific ImGui setup.

## Backend-Specific Code

The following behavior belongs in each backend implementation:

- Native device creation.
- Swapchain creation.
- Resource allocation details.
- Descriptor binding model.
- Command recording implementation.
- Synchronization implementation.
- Pipeline object creation.
- ImGui backend renderer initialization.

## Adding Vulkan Later

Vulkan support should be added by creating a Vulkan implementation of the existing RHI contracts.

The Vulkan backend should not require:

- Rewriting the app layer.
- Rewriting the 2D renderer.
- Rewriting the 3D renderer.
- Exposing Vulkan headers to shared renderer code.

Some backend-specific differences are expected. The RHI should stay practical and should not hide important performance concepts if doing so makes both backends worse.

## ImGui Backend Rule

ImGui panel code can be shared across backends, but renderer/platform binding code is backend-specific.

Expected split:

```txt
DevUI/
  Panels and debug tooling.

RHI_DX12/
  DX12 ImGui renderer binding.

RHI_Vulkan/
  Vulkan ImGui renderer binding, added later.
```

ImGui is compiled only when `NGIN_DEV_UI` is enabled.
