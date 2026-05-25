#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class DocumenVisibilityState : uint8
  {
    Visible,
    Hidden,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::DocumenVisibilityState, 2uz);