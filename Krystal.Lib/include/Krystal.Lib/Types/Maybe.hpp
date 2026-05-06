#pragma once

#include <optional>

namespace Krys
{
  template <typename T>
  using Maybe = std::optional<T>;

  constexpr inline auto Null = std::nullopt;
}