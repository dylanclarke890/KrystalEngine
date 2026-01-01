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

namespace Krys
{
  template <>
  constexpr underlying_t<UI::ExperimentalFeature> OrdinalCount<UI::ExperimentalFeature> = 2;

  template <>
  inline constexpr bool EnableEnumFlags<UI::ExperimentalFeature> = true;
}