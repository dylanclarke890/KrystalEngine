#pragma once

#include "Krystal.Core/Types/String.hpp"
#include <expected>

namespace krys
{
  template <typename T, typename ErrorType = string>
  using Expected = std::expected<T, ErrorType>;

  template <typename T>
  using Unexpected = std::unexpected<T>;
}