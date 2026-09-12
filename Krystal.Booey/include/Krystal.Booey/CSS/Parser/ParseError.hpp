#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class ParseError : uint8
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::ParseError, 8uz);
