#pragma once

#include "Krystal.Lib/String.hpp"
#include <expected>

namespace Krys
{
  template <typename T, typename ErrorType = string>
  using Expected = std::expected<T, ErrorType>;

  template <typename T>
  using Unexpected = std::unexpected<T>;
}