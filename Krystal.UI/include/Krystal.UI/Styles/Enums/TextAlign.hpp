#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class TextAlign : uint8
  {
    Left,
    Center,
    Right,
    Justify
  };
}

namespace Krys
{
  template <>
  KRYS_NODISCARD constexpr uint32 OrdinalCount<UI::TextAlign>() noexcept
  {
    return 4;
  }
}