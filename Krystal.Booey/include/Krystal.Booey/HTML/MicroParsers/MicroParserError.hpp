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
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::MicroParserError, 3uz);