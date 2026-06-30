#pragma once
#include <d3d12.h>
#include <ngin/pch.h>

namespace Ngin {
struct Shader {
  Scope<ID3DBlob> shader;
  const WCHAR* filename;
  const char* entrypoint;
  const char* target;
  UINT flags1;
  UINT flags2;
};
};  // namespace Ngin
