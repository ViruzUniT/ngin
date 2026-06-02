#pragma once

#include <ngin/core/config.h>

#include <cstddef>
#include <cstdint>
#include <format>
#include <source_location>
#include <string_view>
#include <type_traits>
#include <utility>

#if defined(NGIN_PLATFORM_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#endif
