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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::UI::Wrap, 3u)
