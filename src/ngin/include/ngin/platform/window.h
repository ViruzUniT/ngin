#pragma once
#include <ngin/core/base.h>
#include <stdint.h>

namespace Ngin {
namespace Window {

enum CmdShow {
  Hide = 0,
  ShowNormal,
  Show = 5,
};

struct Properties {
  uint16_t width;
  uint16_t height;
  std::string_view name;

  Properties(uint16_t width, uint16_t height, std::string_view name)
      : width(width), height(height), name(name) {}
};

ErrorCode Create(Properties const& props);
ErrorCode SetShow(CmdShow shouldShow);
ErrorCode Update();
}  // namespace Window
}  // namespace Ngin
