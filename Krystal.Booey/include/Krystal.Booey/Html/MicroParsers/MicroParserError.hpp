#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html
{
  enum class MicroParserError
  {
    None,
    UnexpectedEndOfInput,
    InvalidCharacter,
    OutOfRange,
    InvalidSign,
  };

  consteval MicroParserError MaxEnumValue(MicroParserError) noexcept
  {
    return MicroParserError::InvalidSign;
  }
}
