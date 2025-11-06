#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include <compare>

namespace Krys
{
  struct StringRef
  {
    size_t Id {0u};

    constexpr auto operator<=>(const StringRef &other) const noexcept = default;

    constexpr bool IsValid() const noexcept
    {
      return Id != 0u;
    }
  };
}