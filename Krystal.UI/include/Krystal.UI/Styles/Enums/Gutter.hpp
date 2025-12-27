#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class Gutter : uint8
  {
    Row,
    Column,
    All
  };
}

namespace Krys
{
  template <>
  KRYS_NODISCARD constexpr uint32 OrdinalCount<UI::Gutter>() noexcept
  {
    return 3;
  }
}