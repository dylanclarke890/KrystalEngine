#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class PseudoClassId : uint8
  {
    Invalid,
    Lang,
    Host,
    Scope,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::PseudoClassId, 4uz);