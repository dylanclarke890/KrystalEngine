#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class DocumentVisibilityState : uint8
  {
    Visible,
    Hidden,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::DocumentVisibilityState, 2uz);