#pragma once

#include <cstddef>
#include <memory>

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

template <typename T>
using Scope = std::unique_ptr<T>;
template <typename T>
using ComScope = std::unique_ptr<T, ComReleaser>;

template <typename T>
using Ref = std::shared_ptr<T>;

}  // namespace Ngin
