#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class QuirksMode
  {
    NoQuirks,
    Quirks,
    LimitedQuirks
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::QuirksMode, 3uz);