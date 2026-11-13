#pragma once

#include <ngin/pch.h>
#include <ngin/rhi_dx12/rhi.h>

namespace Ngin {

struct DearImGui {
  DearImGui(WeakRef<RHI> wRhi, const HWND hwnd);
  ~DearImGui();

  Error Render();

  ImGuiContext* context;
  ImGuiIO& io;
  WeakRef<RHI> wRhi;
};
}  // namespace Ngin
