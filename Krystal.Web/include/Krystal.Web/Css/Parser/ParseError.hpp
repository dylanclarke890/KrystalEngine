#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::web::css
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

  consteval ParseError MaxEnumValue(ParseError) noexcept
  {
    return ParseError::InvalidEscapeSequence;
  }
}
