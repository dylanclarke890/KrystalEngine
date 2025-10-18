#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Types.hpp"
#include <compare>

namespace Krys::Gfx
{
  template <typename Derived>
  struct Handle
  {
    uint32 Id {0u};

    NO_DISCARD bool IsValid() const noexcept
    {
      return Id != 0u;
    }

    constexpr auto operator<=>(const Handle &other) const noexcept = default;
  };
}