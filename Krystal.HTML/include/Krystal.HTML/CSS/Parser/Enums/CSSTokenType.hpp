#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSTokenType : uint8
  {
    Uninitialized,
    Ident,
    Function,
    AtKeyword,
    Hash,
    String,
    BadString,
    Url,
    BadUrl,
    Delim,
    Number,
    Percentage,
    Dimension,
    Whitespace,
    CDO,
    CDC,
    Colon,
    Semicolon,
    Comma,
    OpenSquare,
    CloseSquare,
    OpenParen,
    CloseParen,
    OpenCurly,
    CloseCurly,
    UnicodeRange,
    EndOfFile
  };

  enum class HashTokenType : uint8
  {
    Unrestricted,
    Id
  };

  enum class NumericValueType : uint8
  {
    Number,
    Integer
  };

  enum class NumericSignChar : uint8
  {
    Missing,
    Plus,
    Minus
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSTokenType, 27uz);

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::HashTokenType, 2uz);

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::NumericValueType, 2uz);

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::NumericSignChar, 3uz);