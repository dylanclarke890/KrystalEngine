#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#insert-adjacent
  enum class InsertAdjacentWhere : uint8
  {
    BeforeBegin,
    AfterBegin,
    BeforeEnd,
    AfterEnd
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::InsertAdjacentWhere, 4uz);