#pragma once

#include <ngin/core/base.h>

#include <cstddef>
#include <cstdint>
#include <format>
#include <source_location>
#include <string_view>
#include <type_traits>
#include <utility>

static constexpr uint8_t FRAME_COUNT = 2;

#if defined(NGIN_PLATFORM_WINDOWS)
#include <ngin/platform/windows.h>
#include <windows.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif
