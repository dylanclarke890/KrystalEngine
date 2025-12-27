#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <compare>

namespace Krys
{
  struct StringRef
  {
    size_t Id {0u};

    constexpr auto operator<=>(const StringRef &other) const noexcept = default;

    KRYS_NODISCARD constexpr bool IsValid() const noexcept
    {
      return Id != 0u;
    }
  };
}