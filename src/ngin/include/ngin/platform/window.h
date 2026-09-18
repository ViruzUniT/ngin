#pragma once
#include <ngin/pch.h>
#include <ngin/rhi_dx12/device.h>
#include <stdint.h>

namespace Ngin {
namespace Window {
enum CmdShow { Hide = 0, ShowNormal = 1, ShowMaximized = 3, Show = 5 };

struct Window {
  std::string name;
  std::string className;
  Vec2Rounded dimensions;
  HWND hwnd = nullptr;
  ATOM w_class = 0;
  bool fullscreen = false;
  RECT windowPos;

  Scope<RHI> rhi;

  Window(uint16_t width, uint16_t height, std::string_view name, std::string_view className)
      : dimensions(width, height), name(name), className(className) {}
  Window() = default;
};

Error Create(Window& window);
Error SetShow(Window& window, CmdShow showType);
Error Update(Window& window);
Error Resize(Window& window, bool autoResize = true, Vec2Rounded newDimensions = Vec2Rounded());
Error SetFullscreen(Window& window, bool enable);
Error Close(Window& window);
}  // namespace Window
}  // namespace Ngin
