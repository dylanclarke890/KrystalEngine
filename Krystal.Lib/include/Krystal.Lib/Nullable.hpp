#pragma once

#include <optional>

namespace Krys
{
  template <typename T>
  using Nullable = std::optional<T>;
}