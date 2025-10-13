#pragma once

#include "Krystal.Lib/Enum.hpp"
#include "Krystal.Lib/Types.hpp"

namespace Krys::UI
{
  enum class Dimension
  {
    Width,
    Height
  };
}

namespace Krys
{
  template <>
  constexpr int32 OrdinalCount<UI::Dimension>()
  {
    return 2;
  }
}