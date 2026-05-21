#include <ngin/core/log.h>

#include <iostream>

namespace Ngin {
namespace {

const char* logLevelName(LogLevel level) {
  switch (level) {
    case LogLevel::Trace:
      return "~";
    case LogLevel::Debug:
      return "/";
    case LogLevel::Info:
      return "+";
    case LogLevel::Warn:
      return "*";
    case LogLevel::Error:
      return "-";
    case LogLevel::Fatal:
      return "!";
  }

  return "unknown";
}

}  // namespace

void log(LogLevel level, std::string_view message) {
  std::ostream& output = (level == LogLevel::Error || level == LogLevel::Fatal) ? std::cerr : std::clog;
  output << '[' << logLevelName(level) << "] " << message << '\n';
}

void logTrace(std::string_view message) { log(LogLevel::Trace, message); }
void logDebug(std::string_view message) { log(LogLevel::Debug, message); }
void logInfo(std::string_view message) { log(LogLevel::Info, message); }
void logWarn(std::string_view message) { log(LogLevel::Warn, message); }
void logError(std::string_view message) { log(LogLevel::Error, message); }
void logFatal(std::string_view message) { log(LogLevel::Fatal, message); }

}  // namespace Ngin
