#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class Wrap : uint8
  {
    NoWrap,
    Wrap,
    WrapReverse,
  };
}

namespace Krys
{
  template <>
  constexpr UnderlyingType<UI::Wrap> OrdinalCount<UI::Wrap> = 3;
}