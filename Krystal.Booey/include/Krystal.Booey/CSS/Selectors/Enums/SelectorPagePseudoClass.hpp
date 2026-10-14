#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  /// @brief The pseudo-classes allowed in an `@page` selector.
  enum class SelectorPagePseudoClass : uint8
  {
    First,
    Left,
    Right,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::SelectorPagePseudoClass, 3uz);
