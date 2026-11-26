#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::web::css
{
  enum class TokenType : uint8
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

    /// @brief An <eof-token> is a conceptual token, not actually produced by the tokenizer, used to indicate
    /// that the token stream has been exhausted.
    EndOfFile,
  };

  consteval TokenType MaxEnumValue(TokenType) noexcept
  {
    return TokenType::EndOfFile;
  }

  enum class HashTokenType : uint8
  {
    Unrestricted,
    Id,
  };

  consteval HashTokenType MaxEnumValue(HashTokenType) noexcept
  {
    return HashTokenType::Id;
  }

  enum class NumericValueType : uint8
  {
    Number,
    Integer,
  };

  consteval NumericValueType MaxEnumValue(NumericValueType) noexcept
  {
    return NumericValueType::Integer;
  }

  enum class NumericSignChar : uint8
  {
    Missing,
    Plus,
    Minus,
  };

  consteval NumericSignChar MaxEnumValue(NumericSignChar) noexcept
  {
    return NumericSignChar::Minus;
  }

  enum class BlockTokenType : uint8
  {
    None,
    Start,
    End,
  };

  consteval BlockTokenType MaxEnumValue(BlockTokenType) noexcept
  {
    return BlockTokenType::End;
  }
}
