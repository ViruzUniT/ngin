#pragma once
#include <ngin/core/base.h>
#include <ngin/pch.h>
#include <stdint.h>

namespace Ngin {
namespace Window {
enum CmdShow {
  Hide = 0,
  ShowNormal = 1,
  Show = 5,
};

struct Properties {
  uint16_t width;
  uint16_t height;
  std::string name;
  std::string className = "NginWindow";
  HWND hwnd = nullptr;

  Properties(uint16_t width, uint16_t height, std::string_view name, std::string_view className)
      : width(width), height(height), name(name), className(className) {}
  Properties() = default;
};

ErrorCode Create(Properties const& props);
ErrorCode SetShow(CmdShow shouldShow);
ErrorCode Update();
}  // namespace Window
}  // namespace Ngin
