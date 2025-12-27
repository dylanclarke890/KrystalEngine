#pragma once

#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Core/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

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
  NO_DISCARD constexpr uint32 OrdinalCount<UI::ExperimentalFeature>() noexcept
  {
    return 2;
  }
}