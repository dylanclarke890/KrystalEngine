#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class PseudoElementId : uint8
  {
    Unknown,
    Before,
    After
  };

  KRYS_NODISCARD constexpr bool IsElementBackedPseudoElement(PseudoElementId pseudoElement) noexcept
  {
    return false;
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::PseudoElementId, 3uz);