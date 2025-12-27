#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

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
  KRYS_NODISCARD constexpr uint32 OrdinalCount<UI::Dimension>() noexcept
  {
    return 2;
  }
}