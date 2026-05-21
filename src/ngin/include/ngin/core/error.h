#pragma once

#include <string_view>
namespace Ngin {

enum class ErrorCode {
  None = 0,
  Unknown,
  InvalidArgument,
  FileNotFound,
  PlatformError,
  GraphicsError,
};

struct Error {
  ErrorCode code = ErrorCode::None;
  std::string_view message = {};
};
}  // namespace Ngin
