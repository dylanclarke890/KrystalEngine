#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSParseError : uint8
  {
    UnexpectedEOFInComment,
    UnexpectedReverseSolidus,
    UnexpectedEOFInString,
    UnexpectedNewlineInString,
    UnexpectedEOFInUrl,
    InvalidCharacterInUrl,
    UnexpectedEOFInEscapeSequence,
    InvalidEscapeSequence,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSParseError, 8uz);
