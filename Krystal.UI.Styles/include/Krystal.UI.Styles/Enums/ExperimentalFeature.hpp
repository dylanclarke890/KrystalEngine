#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI::Styles
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
  constexpr uint32 OrdinalCount<UI::Styles::ExperimentalFeature>()
  {
    return 2;
  }
}