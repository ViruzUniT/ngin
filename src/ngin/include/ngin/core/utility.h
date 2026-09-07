#pragma once

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
}  // namespace Ngin
