#pragma once
#include <ngin/pch.h>

#include <cstdlib>
#include <filesystem>
#include <string_view>

namespace Ngin {
struct Shader : NonCopyableNonMovable {
  Shader(std::string_view name);
  inline ~Shader() {
    if (data)
      free(data);
  }

  inline const void* GetBuffer() const { return data; };
  inline size_t GetSize() const { return size; };

 private:
  static std::filesystem::path shaderDir;

  void* data = nullptr;
  size_t size = 0;
};
}  // namespace Ngin
