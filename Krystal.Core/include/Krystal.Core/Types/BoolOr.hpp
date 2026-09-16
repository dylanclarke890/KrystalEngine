#pragma once

#include "Krystal.Core/Types/Variant.hpp"

namespace krys
{
  template <typename... Ts>
  using BoolOr = Variant<bool, Ts...>;
}