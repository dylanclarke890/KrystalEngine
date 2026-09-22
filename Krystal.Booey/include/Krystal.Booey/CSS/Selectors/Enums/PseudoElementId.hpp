#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class PseudoElementId : uint8
  {
    Unknown,
    Before,
    After
  };

  KRYS_NODISCARD constexpr bool IsElementBackedPseudoElement(PseudoElementId pseudoElement) noexcept
  {
    (void)pseudoElement;
    krys_not_implemented();
  }
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::PseudoElementId, 3uz);