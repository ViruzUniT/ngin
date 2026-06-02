#include <ngin/core/assert.h>
#include <ngin/core/log.h>
#include <ngin/pch.h>

#include <cstdlib>
#include <sstream>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace Ngin {

void debugBreak() {
#if defined(_MSC_VER)
  __debugbreak();
#elif defined(__clang__) && defined(__has_builtin)
#if __has_builtin(__builtin_debugtrap)
  __builtin_debugtrap();
#else
  __builtin_trap();
#endif
#elif defined(__GNUC__)
  __builtin_trap();
#else
  std::abort();
#endif
}

[[noreturn]] void reportAssertionFailure(std::string_view expression, std::string_view message,
    std::source_location location) {
  std::ostringstream output;
  output << "Assertion failed: " << expression;

  if (!message.empty()) {
    output << " (" << message << ")";
  }

  output << " at " << location.file_name() << ':' << location.line();

  log(LogLevel::Fatal, output.str());
  debugBreak();
  std::abort();
}

}  // namespace Ngin
