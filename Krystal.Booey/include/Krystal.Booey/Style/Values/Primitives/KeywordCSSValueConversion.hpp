#pragma once

#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  template <typename T>
  requires IsEnum<T>
  struct CSSValueConversion<T>
  {
    KRYS_NODISCARD T operator()(BuilderState &, const css::Value &value) noexcept
    {
      return FromValueId<T>(value.ValueId());
    }
  };
}
