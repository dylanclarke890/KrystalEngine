#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#canplaytyperesult
  enum class CanPlayTypeResult : uint8
  {
    None,
    Probably,
    Maybe,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CanPlayTypeResult, 3uz);