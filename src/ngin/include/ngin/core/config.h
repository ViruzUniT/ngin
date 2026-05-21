#pragma once

// Build configuration --------------------------------------------------------

#if (defined(NGIN_DEBUG) + defined(NGIN_STAGING) + defined(NGIN_RELEASE)) != 1
#error "Exactly one build configuration must be defined: NGIN_DEBUG, NGIN_STAGING, or NGIN_RELEASE."
#endif

#if defined(NGIN_DEV_UI)
#define NGIN_HAS_DEV_UI 1
#else
#define NGIN_HAS_DEV_UI 0
#endif

#if defined(NGIN_RELEASE) && NGIN_HAS_DEV_UI
#error "NGIN_DEV_UI must not be enabled in RELEASE builds."
#endif

// Platform -------------------------------------------------------------------

#if defined(_WIN32)
#define NGIN_PLATFORM_WINDOWS 1
#else
#error "Unsupported platform. Ngin currently targets Windows first."
#endif

// Compiler -------------------------------------------------------------------

#if defined(__clang__)
#define NGIN_COMPILER_CLANG 1
#elif defined(_MSC_VER)
#define NGIN_COMPILER_MSVC 1
#else
#error "Unsupported compiler. Use Clang or MSVC."
#endif

// Architecture ---------------------------------------------------------------

#if defined(_M_X64) || defined(__x86_64__)
#define NGIN_ARCH_X64 1
#else
#error "Unsupported architecture. Ngin currently targets x64."
#endif

// C++ standard ---------------------------------------------------------------

#if defined(_MSVC_LANG)
#define NGIN_CPP_VERSION _MSVC_LANG
#else
#define NGIN_CPP_VERSION __cplusplus
#endif

#if NGIN_CPP_VERSION < 202302L
#error "Ngin requires C++23 or newer."
#endif
