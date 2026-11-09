// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016-2024 Apple Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/NumericValue.hpp"
#include "Krystal.Booey/CSS/Parser/ParseError.hpp"
#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  struct TokenizerError
  {
    ParseError Error;
    SourceLocation Location {};
  };

  /// @see https://drafts.csswg.org/css-syntax/#tokenization
  class Tokenizer
  {
  private:
    InputStream _inputStream;
    List<Token> _tokens;
    List<TokenizerError> _errors;
    SmallList<TokenType> _blockStack;

  public:
    explicit Tokenizer(InputStream &inputStream) noexcept;

    /// @brief Pumps the tokenizer until the input stream is exhausted. Returns true if the tokenizer has
    /// consumed at least one token.
    bool PumpTokenizer() noexcept;

    /// @brief Gets the range of tokens produced by the tokenizer.
    /// @note The tokens are only valid as long as the tokenizer is alive. The tokenizer owns the tokens and
    /// will free them when it is destroyed.
    KRYS_NODISCARD TokenRange Tokens() const noexcept
    {
      return TokenRange(_tokens);
    }

    /// @brief Gets the number of tokens produced by the tokenizer.
    KRYS_NODISCARD size_t TokenCount() const noexcept
    {
      return _tokens.size();
    }

    /// @brief Gets the list of errors produced by the tokenizer.
    KRYS_NODISCARD const List<TokenizerError> &Errors() noexcept
    {
      return _errors;
    }

  private:
    /// @see https://drafts.csswg.org/css-syntax/#tokenizer-consume-a-token
    KRYS_NODISCARD Token ConsumeToken(bool unicodeRangesAllowed = false) noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-comments
    void ConsumeComments() noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-a-numeric-token
    Token ConsumeNumericToken() noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-an-ident-like-token
    Token ConsumeIdentLikeToken() noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-a-string-token
    Token ConsumeStringToken(char32 endingCodePoint) noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-a-url-token
    Token ConsumeUrlToken() noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-an-escaped-code-point
    KRYS_NODISCARD char32 ConsumeEscapedCodePoint() noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#starts-with-a-valid-escape
    KRYS_NODISCARD bool StartsWithValidEscape(char32 first, char32 second) const noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#check-if-three-code-points-would-start-an-ident-sequence
    KRYS_NODISCARD bool StartsWithIdentifier(char32 first, char32 second, char32 third) const noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#check-if-three-code-points-would-start-a-number
    KRYS_NODISCARD bool StartsWithNumber(char32 first, char32 second, char32 third) const noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#check-if-three-code-points-would-start-a-unicode-range
    KRYS_NODISCARD bool StartsWithUnicodeRange(char32 first, char32 second, char32 third) const noexcept;

    /// @brief Helper function to check if the next characters in the input stream start with an exponent,
    /// which is part of a number token.
    KRYS_NODISCARD bool StartsWithExponent(char32 first, char32 second, char32 third) const noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-an-ident-sequence
    KRYS_NODISCARD utf32_string ConsumeIdentSequence() noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-a-number
    KRYS_NODISCARD NumericValue ConsumeNumber() noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-a-unicode-range-token
    KRYS_NODISCARD Token ConsumeUnicodeRangeToken() noexcept;

    /// @see https://drafts.csswg.org/css-syntax/#consume-the-remnants-of-a-bad-url
    void ConsumeRemnantsOfBadUrl() noexcept;

    /// @brief Utility helper for consuming whitespace from the input stream without emitting whitespace
    /// tokens, as required by some parts of the spec.
    void ConsumeWhitespace() noexcept;

    /// @brief Reports an error that occurred during tokenization.
    void ParseError(ParseError error) noexcept;

    /// @brief Creates a Token for a block start token (e.g., `{`, `[`, `(`).
    KRYS_NODISCARD Token BlockStart(TokenType type) noexcept;

    /// @brief Creates a Token for a named block start token (e.g. a function token), which opens a block
    /// of `blockType`.
    KRYS_NODISCARD Token BlockStart(TokenType blockType, TokenType type, utf32_stringview name) noexcept;

    /// @brief Creates a Token for a block start token (e.g., `}`, `]`, `)`). May not actually be classed
    /// as a block end token depending on the current block stack.
    KRYS_NODISCARD Token BlockEnd(TokenType type) noexcept;

#pragma region Tokenizer Definitions - https://drafts.csswg.org/css-syntax/#tokenizer-definitions

    /// @see https://drafts.csswg.org/css-syntax/#digit
    KRYS_NODISCARD static bool IsDigit(char32 codePoint) noexcept
    {
      return krys::text::IsASCIIDigit(codePoint);
    }

    /// @see https://drafts.csswg.org/css-syntax/#hex-digit
    KRYS_NODISCARD static bool IsHexDigit(char32 codePoint) noexcept
    {
      return krys::text::IsASCIIHexDigit(codePoint);
    }

    /// @see https://drafts.csswg.org/css-syntax/#uppercase-letter
    KRYS_NODISCARD static bool IsUppercaseLetter(char32 codePoint) noexcept
    {
      return krys::text::IsASCIIUpper(codePoint);
    }

    /// @see https://drafts.csswg.org/css-syntax/#lowercase-letter
    KRYS_NODISCARD static bool IsLowercaseLetter(char32 codePoint) noexcept
    {
      return krys::text::IsASCIILower(codePoint);
    }

    /// @see https://drafts.csswg.org/css-syntax/#letter
    KRYS_NODISCARD static bool IsLetter(char32 codePoint) noexcept
    {
      return krys::text::IsASCIIAlpha(codePoint);
    }

    /// @see https://drafts.csswg.org/css-syntax/#non-ascii-code-point
    KRYS_NODISCARD static bool IsNonASCIICodePoint(char32 codePoint) noexcept
    {
      return codePoint == U'\u00B7' || (codePoint >= U'\u00C0' && codePoint <= U'\u00D6')
             || (codePoint >= U'\u00D8' && codePoint <= U'\u00F6')
             || (codePoint >= U'\u00F8' && codePoint <= U'\u037D')
             || (codePoint >= U'\u037F' && codePoint <= U'\u1FFF') || codePoint == U'\u200C'
             || codePoint == U'\u200D' || codePoint == U'\u203F' || codePoint == U'\u2040'
             || (codePoint >= U'\u2070' && codePoint <= U'\u218F')
             || (codePoint >= U'\u2C00' && codePoint <= U'\u2FEF')
             || (codePoint >= U'\u3001' && codePoint <= U'\uD7FF')
             || (codePoint >= U'\uF900' && codePoint <= U'\uFDCF')
             || (codePoint >= U'\uFDF0' && codePoint <= U'\uFFFD') || codePoint >= U'\U00010000';
    }

    /// @see https://drafts.csswg.org/css-syntax/#ident-start-code-point
    KRYS_NODISCARD static bool IsIdentStartCodePoint(char32 codePoint) noexcept
    {
      return IsLetter(codePoint) || IsNonASCIICodePoint(codePoint) || codePoint == U'_';
    }

    /// @see https://drafts.csswg.org/css-syntax/#ident-code-point
    KRYS_NODISCARD static bool IsIdentCodePoint(char32 codePoint) noexcept
    {
      return IsIdentStartCodePoint(codePoint) || IsDigit(codePoint) || codePoint == U'-';
    }

    /// @see https://drafts.csswg.org/css-syntax/#non-printable-code-point
    KRYS_NODISCARD static bool IsNonPrintableCodePoint(char32 codePoint) noexcept
    {
      return (codePoint >= 0x0 && codePoint <= 0x8) || codePoint == 0xB
             || (codePoint >= 0xE && codePoint <= 0x1F) || codePoint == 0x7F;
    }

    /// @see https://drafts.csswg.org/css-syntax/#newline
    KRYS_NODISCARD static bool IsNewline(char32 codePoint) noexcept
    {
      return codePoint == U'\n';
    }

    /// @see https://drafts.csswg.org/css-syntax/#whitespace
    KRYS_NODISCARD static bool IsWhitespace(char32 codePoint) noexcept
    {
      return IsNewline(codePoint) || codePoint == U'\t' || codePoint == U' ';
    }

#pragma endregion
  };
}