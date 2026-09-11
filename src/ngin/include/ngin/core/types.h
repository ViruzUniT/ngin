#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "ngin/core/utility.h"

struct ComReleaser {
  template <typename T>
  void operator()(T* ptr) const {
    if (ptr) {
      ptr->Release();
    }
  }
};

namespace Ngin {
using f32 = float;
using f64 = double;

// template <typename T>
// using Scope = std::unique_ptr<T>;

template <typename T>
struct ComScope : NonCopyable {
 public:
  ComScope() noexcept = default;
  ~ComScope() { reset(); }

  template <typename... Args>
  ComScope(Args&&... args) {
    ptr = new T(std::forward<Args>(args)...);
  }

  ComScope(T*& ptr) {
    this->ptr = std::move(ptr);
    ptr = nullptr;
  }

  ComScope(ComScope&& other) noexcept : ptr(other.ptr) { other.ptr = nullptr; }

  inline ComScope& operator=(ComScope&& other) noexcept {
    reset();
    ptr = other.ptr;
    other.ptr = nullptr;
    return *this;
  }

  inline void* operator new(size_t) = delete;
  inline void* operator new[](size_t) = delete;

  inline void operator=(T*&) = delete;
  inline void operator=(T* const&) = delete;

  inline T** operator&() noexcept { return &ptr; }
  inline T* operator->() noexcept { return ptr; }

  inline T* const get() const noexcept { return ptr; }

  explicit operator bool() const noexcept { return ptr != nullptr; }

  operator T*() const noexcept { return get(); }

  T** put() noexcept {
    reset();
    return &ptr;
  }

  void reset() noexcept {
    if (ptr) {
      ptr->Release();
      ptr = nullptr;
    }
  }

  T* detach() noexcept {
    T* result = ptr;
    ptr = nullptr;
    return result;
  }

 private:
  T* ptr = nullptr;
};

// template <typename T>
// using ComScope = std::unique_ptr<T, ComReleaser>;

template <typename T>
using Scope = std::unique_ptr<T>;
template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T>
using List = std::vector<T>;

template <typename T, std::size_t N>
using Array = std::array<T, N>;

}  // namespace Ngin
