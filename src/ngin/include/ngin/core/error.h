#pragma once

#include <string_view>
namespace Ngin {

enum ErrorCode {
  Unknown = -1,
  None = 0,
  Exit = 2,
  InvalidArgument,
  FileNotFound,
  PlatformError,
  GraphicsError,
  FenceError,
  FenceTimeout
};

struct Error {
  ErrorCode code = ErrorCode::None;
  std::string_view message = {};
};
}  // namespace Ngin
