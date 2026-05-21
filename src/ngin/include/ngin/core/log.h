#pragma once

#include <string_view>

namespace Ngin {

enum class LogLevel {
  Trace,
  Debug,
  Info,
  Warn,
  Error,
  Fatal,
};

void log(LogLevel level, std::string_view message);

void logTrace(std::string_view message);
void logDebug(std::string_view message);
void logInfo(std::string_view message);
void logWarn(std::string_view message);
void logError(std::string_view message);
void logFatal(std::string_view message);

}  // namespace Ngin
