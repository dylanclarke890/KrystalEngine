#pragma once

#include "Krystal.Core/Types/Null.hpp"
#include <optional>

namespace krys
{
  template <typename T>
  using Maybe = std::optional<T>;
}