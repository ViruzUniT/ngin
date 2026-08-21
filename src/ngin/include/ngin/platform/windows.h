#pragma once
#include <Windows.h>

#include <filesystem>

namespace Ngin {
inline std::filesystem::path getExecutableDirectory() {
  wchar_t buffer[MAX_PATH];
  GetModuleFileNameW(nullptr, buffer, MAX_PATH);

  return std::filesystem::path(buffer).parent_path();
}
}  // namespace Ngin
