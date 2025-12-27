#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Enum.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::UI
{
  enum class Display : uint8
  {
    Flex,
    None,
    Contents
  };
}

namespace Krys
{
  template <>
  KRYS_NODISCARD constexpr uint32 OrdinalCount<UI::Display>() noexcept
  {
    return 3;
  }
}