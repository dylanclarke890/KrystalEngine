#pragma once

#include <optional>

namespace krys
{
  template <typename T>
  using Maybe = std::optional<T>;

  constexpr auto null = std::nullopt;
}