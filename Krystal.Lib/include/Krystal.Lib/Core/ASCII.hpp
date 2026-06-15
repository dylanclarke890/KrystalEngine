#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys
{
  KRYS_NODISCARD constexpr bool IsASCII(char8 c) noexcept
  {
    return static_cast<uchar>(c) <= 0x7F;
  }

  
  KRYS_NODISCARD constexpr bool ContainsOnlyASCII(utf8_stringview str) noexcept
  {
    for (auto c : str)
    {
      if (c > 0x7Fu)
      {
        return false;
      }
    }

    return true;
  }
}