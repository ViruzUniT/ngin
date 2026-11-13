#pragma once
#include <ngin/pch.h>
#include <ngin/renderer/imgui.h>
#include <ngin/rhi_dx12/rhi.h>
#include <stdint.h>

#include <unordered_map>

namespace Ngin {

struct Window {
  std::string name;
  std::string className;
  Vec2Rounded dimensions;
  HWND hwnd = nullptr;
  ATOM w_class = 0;
  bool fullscreen = false;
  RECT windowPos;
  Scope<DearImGui> imgui;

  Ref<RHI> rhi;

  enum CmdShow { Hide = 0, ShowNormal = 1, ShowMaximized = 3, Show = 5 };

  static std::unordered_map<HWND, Window*> windows;
  static std::vector<HWND> windowsToClose;

  Window(uint16_t width, uint16_t height, std::string_view name, std::string_view className);
  ~Window() {
    imgui.reset();
    logTrace("Desturcting window");
    Close();
  }
  Error SetShow(CmdShow showType);
  Error Update();
  Error Resize(bool autoResize = true, std::optional<Vec2Rounded> newDimensions = {});
  Error SetFullscreen(bool enable);
  Error Close();
  static Error Close(Window& window);
  static Window* GetWindow(HWND hwnd);
  static void CheckForClosableWindows();
};
}  // namespace Ngin
