#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class ExperimentalFeature
  {
    WebFlexBasis,
  };
}

namespace Krys
{
  template <>
  constexpr int32 OrdinalCount<UI::ExperimentalFeature>()
  {
    return 1;
  }
}