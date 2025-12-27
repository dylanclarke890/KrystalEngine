#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class ExperimentalFeature : uint8
  {
    None,
    WebFlexBasis,
  };

  KRYS_ENUM_FLAG_OPERATORS(ExperimentalFeature)
}

namespace Krys
{
  template <>
  KRYS_NODISCARD constexpr uint32 OrdinalCount<UI::ExperimentalFeature>() noexcept
  {
    return 2;
  }
}