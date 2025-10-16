#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class Dimension : uint8
  {
    Width,
    Height
  };
}

namespace Krys
{
  template <>
  constexpr uint32 OrdinalCount<UI::Dimension>()
  {
    return 2;
  }
}