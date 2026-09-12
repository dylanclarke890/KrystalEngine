#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#canplaytyperesult
  enum class CanPlayTypeResult : uint8
  {
    None,
    Probably,
    Maybe,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::CanPlayTypeResult, 3uz);