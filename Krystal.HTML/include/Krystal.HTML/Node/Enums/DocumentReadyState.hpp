#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class DocumentReadyState : uint8
  {
    Loading,
    Interactive,
    Complete,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::DocumentReadyState, 3uz);