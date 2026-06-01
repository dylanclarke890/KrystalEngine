#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML::MicroParsers
{
  enum class MicroParserError
  {
    None,
    UnexpectedEndOfInput,
    InvalidCharacter,
    OutOfRange,
    InvalidSign,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::MicroParsers::MicroParserError, 3uz);