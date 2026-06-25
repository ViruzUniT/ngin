#pragma once
#include <ngin/pch.h>

namespace Ngin {
namespace Renderer {
void Initialize(HWND handle, uint16_t width, uint16_t height);
void RenderFrame();
void Resize(uint16_t width, uint16_t height);
}  // namespace Renderer
}  // namespace Ngin
