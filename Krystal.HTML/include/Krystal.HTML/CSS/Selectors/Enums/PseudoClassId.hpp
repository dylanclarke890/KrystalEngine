#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class PseudoClassId : uint8
  {
    Invalid,
    Lang,
    Host,
    Scope,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::PseudoClassId, 4uz);