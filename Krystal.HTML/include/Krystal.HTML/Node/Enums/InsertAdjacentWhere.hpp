#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class InsertAdjacentWhere : uint8
  {
    BeforeBegin,
    AfterBegin,
    BeforeEnd,
    AfterEnd
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::InsertAdjacentWhere, 4uz);