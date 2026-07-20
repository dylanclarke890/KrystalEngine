#pragma once

#include "Krystal.HTML/CSS/Parser/CSSInputStream.hpp"
#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.HTML/CSS/Parser/Enums/CSSParseError.hpp"
#include "Krystal.HTML/CSS/Parser/Types/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Parser/Types/ParsedInt64OrDouble.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  struct CSSTokenizerError
  {
    CSSParseError Error;
    SourceLocation Location {};
  };

  /// @see https://www.w3.org/TR/css-syntax-3/#tokenization
  class CSSTokenizer
  {
  private:
    CSSInputStream _inputStream;
    List<CSSToken> _tokens;
    List<CSSTokenizerError> _errors;

  public:
    explicit CSSTokenizer(CSSInputStream &inputStream) noexcept;

    /// @brief Pumps the tokenizer until the input stream is exhausted. Returns true if the tokenizer has
    /// consumed at least one token.
    bool PumpTokenizer() noexcept;

    /// @brief Gets the range of tokens produced by the tokenizer.
    /// @note The tokens are only valid as long as the tokenizer is alive. The tokenizer owns the tokens and
    /// will free them when it is destroyed.
    KRYS_NODISCARD CSSTokenRange TokenRange() const noexcept
    {
      return CSSTokenRange(_tokens);
    }

    /// @brief Gets the number of tokens produced by the tokenizer.
    KRYS_NODISCARD size_t TokenCount() const noexcept
    {
      return _tokens.size();
    }

    /// @brief Gets the list of errors produced by the tokenizer.
    KRYS_NODISCARD const List<CSSTokenizerError> &Errors() noexcept
    {
      return _errors;
    }

  private:
    /// @see https://www.w3.org/TR/css-syntax-3/#consume-token
    KRYS_NODISCARD CSSToken ConsumeToken() noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#consume-comment
    void ConsumeComments() noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#consume-numeric-token
    CSSToken ConsumeNumericToken() noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#consume-ident-like-token
    CSSToken ConsumeIdentLikeToken() noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#consume-string-token
    CSSToken ConsumeStringToken(char32 endingCodePoint) noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#consume-url-token
    CSSToken ConsumeUrlToken() noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#consume-escaped-code-point
    KRYS_NODISCARD char32 ConsumeEscapedCodePoint() noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#starts-with-a-valid-escape
    KRYS_NODISCARD bool StartsWithValidEscape(char32 first, char32 second) const noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#would-start-an-identifier
    KRYS_NODISCARD bool WouldStartIdentifier(char32 first, char32 second, char32 third) const noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#starts-with-a-number
    KRYS_NODISCARD bool StartsWithNumber(char32 first, char32 second, char32 third) const noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#consume-ident-sequence
    utf32_string ConsumeIdentSequence() noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#consume-number
    KRYS_NODISCARD ParsedInt64OrDouble ConsumeNumber() noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#convert-string-to-number
    KRYS_NODISCARD ParsedInt64OrDouble ConvertStringToNumber(utf32_stringview codePoints,
                                                             NumericTokenType type) noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#consume-remnants-of-bad-url
    void ConsumeRemnantsOfBadUrl() noexcept;

    /// @brief Utility helper for consuming whitespace from the input stream without emitting whitespace
    /// tokens, as required by some parts of the spec.
    void ConsumeWhitespace() noexcept;

    /// @brief Reports an error that occurred during tokenization.
    void ParseError(CSSParseError error) noexcept;

#pragma region Tokenizer Definitions - https://drafts.csswg.org/css-syntax/#tokenizer-definitions

    /// @see https://drafts.csswg.org/css-syntax/#digit
    KRYS_NODISCARD static bool IsDigit(char32 codePoint) noexcept
    {
      return Krys::Text::IsASCIIDigit(codePoint);
    }

    /// @see https://drafts.csswg.org/css-syntax/#hex-digit
    KRYS_NODISCARD static bool IsHexDigit(char32 codePoint) noexcept
    {
      return Krys::Text::IsASCIIHexDigit(codePoint);
    }

    /// @see https://drafts.csswg.org/css-syntax/#uppercase-letter
    KRYS_NODISCARD static bool IsUppercaseLetter(char32 codePoint) noexcept
    {
      return Krys::Text::IsASCIIUpper(codePoint);
    }

    /// @see https://drafts.csswg.org/css-syntax/#lowercase-letter
    KRYS_NODISCARD static bool IsLowercaseLetter(char32 codePoint) noexcept
    {
      return Krys::Text::IsASCIILower(codePoint);
    }

    /// @see https://drafts.csswg.org/css-syntax/#letter
    KRYS_NODISCARD static bool IsLetter(char32 codePoint) noexcept
    {
      return Krys::Text::IsASCIIAlpha(codePoint);
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