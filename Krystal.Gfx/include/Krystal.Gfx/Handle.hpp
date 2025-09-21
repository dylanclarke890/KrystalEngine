#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Lib/Concepts.hpp"

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
  };
}