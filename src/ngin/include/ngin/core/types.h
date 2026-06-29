#pragma once

#include <cstddef>
#include <memory>

namespace Ngin {
using f32 = float;
using f64 = double;

template <typename T>
using Scope = std::unique_ptr<T>;
template <typename T>
using Ref = std::shared_ptr<T>;

}  // namespace Ngin
