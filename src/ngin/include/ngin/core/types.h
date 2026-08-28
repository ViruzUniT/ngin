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
struct Scope : NonCopyable {
 public:
  template <typename... Args>
  Scope(Args&&... args) {
    ptr = new T(std::forward<Args>(args)...);
  }
  Scope(T*& ptr) {
    this->ptr = std::move(ptr);
    ptr = nullptr;
  }
  Scope() = delete;
  inline void* operator new(size_t) = delete;
  inline void* operator new[](size_t) = delete;

  Scope(T* const& ptr) { this->ptr = std::move(ptr); }

  ~Scope() { delete ptr; }

  inline void operator=(T*&) = delete;
  inline void operator=(T* const&) = delete;

  inline T** operator&() noexcept { return &ptr; }
  inline T* operator->() noexcept { return ptr; }

  inline void reset() {
    delete ptr;
    ptr = nullptr;
  }
  inline void reset(T& ptr) {
    delete ptr;
    ptr = nullptr;
  }
  inline T* const get() { return ptr; }

 private:
  T* ptr;
};

template <typename T>
using ComScope = std::unique_ptr<T, ComReleaser>;

template <typename T>
using Ref = std::shared_ptr<T>;

template <typename T>
using List = std::vector<T>;

}  // namespace Ngin
