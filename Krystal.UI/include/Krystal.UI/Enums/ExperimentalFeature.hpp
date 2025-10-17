#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class ExperimentalFeature : uint8
  {
    None,
    WebFlexBasis,
  };

  ENUM_BITWISE_OPERATORS(ExperimentalFeature)
}

namespace Krys
{
  template <>
  constexpr uint32 OrdinalCount<UI::ExperimentalFeature>()
  {
    return 2;
  }
}