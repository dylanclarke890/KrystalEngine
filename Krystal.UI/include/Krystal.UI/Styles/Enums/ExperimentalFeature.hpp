#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class ExperimentalFeature : uint8
  {
    None,
    WebFlexBasis = 1 << 0,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::UI::ExperimentalFeature, 2u)
