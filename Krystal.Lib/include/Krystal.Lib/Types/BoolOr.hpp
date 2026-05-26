#pragma once

#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys::HTML
{
  template <typename... Ts>
  using BoolOr = Variant<bool, Ts...>;
}