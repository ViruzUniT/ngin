#pragma once
#include <ngin/core/base.h>
#include <ngin/pch.h>
#include <stdint.h>

#include "ngin/rhi_dx12/device.h"

namespace Ngin {
namespace Window {
enum CmdShow {
  Hide = 0,
  ShowNormal = 1,
  Show = 5,
};

struct Window {
  uint16_t width;
  uint16_t height;
  std::string name;
  std::string className;
  HWND handle = nullptr;
  ATOM w_class = 0;

  Scope<RHI> rhi;

  Window(uint16_t width, uint16_t height, std::string_view name, std::string_view className)
      : width(width), height(height), name(name), className(className) {}
  Window() = default;
};

Error Create(Window& window);
Error SetShow(Window& window, CmdShow shouldShow);
Error Update(Window& window);
Error Close(Window& window);
}  // namespace Window
}  // namespace Ngin
