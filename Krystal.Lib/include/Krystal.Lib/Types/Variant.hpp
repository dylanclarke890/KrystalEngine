#pragma once

#include <variant>

namespace Krys
{
  template <typename... Types>
  using Variant = std::variant<Types...>;
}