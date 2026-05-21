#pragma once
#include <ngin/core/config.h>

#include <source_location>
#include <string_view>

namespace Ngin {

void debugBreak();

[[noreturn]] void reportAssertionFailure(std::string_view expression, std::string_view message,
    std::source_location location = std::source_location::current());

}  // namespace Ngin

#if defined(NGIN_DEBUG) || defined(NGIN_STAGING)
#define NGIN_ASSERT(condition)                                                         \
  do {                                                                                 \
    if (!(condition)) {                                                                \
      ::Ngin::reportAssertionFailure(#condition, {}, std::source_location::current()); \
    }                                                                                  \
  } while (false)
#define NGIN_ASSERT_MSG(condition, message)                                                   \
  do {                                                                                        \
    if (!(condition)) {                                                                       \
      ::Ngin::reportAssertionFailure(#condition, (message), std::source_location::current()); \
    }                                                                                         \
  } while (false)
#define NGIN_VERIFY(condition) NGIN_ASSERT(condition)
#else
#define NGIN_ASSERT(condition) ((void)0)
#define NGIN_ASSERT_MSG(condition, message) ((void)0)
#define NGIN_VERIFY(condition) ((void)(condition))
#endif

#define NGIN_DEBUG_BREAK() ::Ngin::debugBreak()
