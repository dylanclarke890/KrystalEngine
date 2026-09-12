#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html::MicroParsers
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::MicroParsers::MicroParserError, 3uz);