#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class BoxSizing : uint8
  {
    BorderBox,
    ContentBox,
  };
}

namespace Krys
{
  template <>
  NO_DISCARD constexpr uint32 OrdinalCount<UI::BoxSizing>() noexcept
  {
    return 2;
  }
}