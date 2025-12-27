#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <compare>

namespace Krys::Gfx
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