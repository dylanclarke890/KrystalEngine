#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::UI
{
  enum class ExperimentalFeature : uint8
  {
    None,
    WebFlexBasis = 1 << 0,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::UI::ExperimentalFeature, 2u)
