#pragma once

#include <ngin/core/log.h>
#include <ngin/pch.h>

#include <format>
#include <string_view>
namespace Ngin {
enum ErrorCode {
  CouldBeAProblem = -1,
  None = 0,
  Exit,
  Unknown,
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
  inline explicit operator bool() const noexcept {
    if (code == CouldBeAProblem)
      Ngin::logDebug(std::format("A potential error has occurred: {}", message));
    return code > 0;
  }
};
}  // namespace Ngin
