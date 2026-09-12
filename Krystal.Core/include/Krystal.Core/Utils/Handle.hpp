#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <compare>

namespace krys::Gfx
{
  template <typename Derived>
  struct Handle
  {
    uint32 Id {0u};

    KRYS_NODISCARD bool IsValid() const noexcept
    {
      return Id != 0u;
    }

    constexpr auto operator<=>(const Handle &other) const noexcept = default;
  };
}