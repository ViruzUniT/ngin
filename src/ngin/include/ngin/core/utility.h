#pragma once

#include <type_traits>
#include <utility>

namespace Ngin {
class NonCopyable {
 protected:
  constexpr NonCopyable() noexcept = default;
  ~NonCopyable() = default;

 public:
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
  NonCopyable(NonCopyable&&) noexcept = default;
  NonCopyable& operator=(NonCopyable&&) noexcept = default;
};

class NonMovable {
 protected:
  constexpr NonMovable() noexcept = default;
  ~NonMovable() = default;

 public:
  NonMovable(const NonMovable&) = default;
  NonMovable& operator=(const NonMovable&) = default;
  NonMovable(NonMovable&&) = delete;
  NonMovable& operator=(NonMovable&&) = delete;
};

class NonCopyableNonMovable {
 protected:
  constexpr NonCopyableNonMovable() noexcept = default;
  ~NonCopyableNonMovable() = default;

 public:
  NonCopyableNonMovable(const NonCopyableNonMovable&) = delete;
  NonCopyableNonMovable& operator=(const NonCopyableNonMovable&) = delete;
  NonCopyableNonMovable(NonCopyableNonMovable&&) = delete;
  NonCopyableNonMovable& operator=(NonCopyableNonMovable&&) = delete;
};

template <typename Function>
class ScopeExit {
 public:
  explicit ScopeExit(Function function) noexcept(std::is_nothrow_move_constructible_v<Function>)
      : function_(std::move(function)) {}

  ScopeExit(ScopeExit&& other) noexcept(std::is_nothrow_move_constructible_v<Function>)
      : function_(std::move(other.function_)), active_(std::exchange(other.active_, false)) {}

  ~ScopeExit() noexcept(noexcept(std::declval<Function&>()())) {
    if (active_) {
      function_();
    }
  }

  ScopeExit(const ScopeExit&) = delete;
  ScopeExit& operator=(const ScopeExit&) = delete;
  ScopeExit& operator=(ScopeExit&&) = delete;

  void release() noexcept { active_ = false; }

 private:
  Function function_;
  bool active_ = true;
};

template <typename Function>
ScopeExit(Function) -> ScopeExit<Function>;

template <typename Enum>
constexpr std::underlying_type_t<Enum> toUnderlying(Enum value) noexcept {
  static_assert(std::is_enum_v<Enum>, "toUnderlying requires an enum type.");
  return static_cast<std::underlying_type_t<Enum>>(value);
}

}  // namespace Ngin
