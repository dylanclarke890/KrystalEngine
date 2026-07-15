#pragma once

#include "Krystal.HTML/CSS/Parser/CSSInputStream.hpp"
#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.HTML/CSS/Parser/Enums/CSSParseError.hpp"
#include "Krystal.HTML/CSS/Parser/Types/ParsedInt64OrDouble.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class CSSTokenizer
  {
  public:
    struct CSSTokenizerError
    {
      CSSParseError Error;
      SourceLocation Location {};
    };

  private:
    CSSInputStream _inputStream;
    List<CSSTokenizerError> _parseErrors;

  public:
    explicit CSSTokenizer(CSSInputStream &inputStream) noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#consume-token
    KRYS_NODISCARD CSSToken ConsumeToken() noexcept;

    KRYS_NODISCARD const List<CSSTokenizerError> ParseErrors() noexcept
    {
      return _parseErrors;
    }

  private:
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

    void ConsumeWhitespace() noexcept;

    void ParseError(CSSParseError error) noexcept;

#pragma region Tokenizer Definitions - https://www.w3.org/TR/css-syntax-3/#tokenizer-definitions

    /// @see https://www.w3.org/TR/css-syntax-3/#digit
    KRYS_NODISCARD static bool IsDigit(char32 codePoint) noexcept
    {
      return Krys::Text::IsASCIIDigit(codePoint);
    }

    /// @see https://www.w3.org/TR/css-syntax-3/#hex-digit
    KRYS_NODISCARD static bool IsHexDigit(char32 codePoint) noexcept
    {
      return Krys::Text::IsASCIIHexDigit(codePoint);
    }

    /// @see https://www.w3.org/TR/css-syntax-3/#uppercase-letter
    KRYS_NODISCARD static bool IsUppercaseLetter(char32 codePoint) noexcept
    {
      return Krys::Text::IsASCIIUpper(codePoint);
    }

    /// @see https://www.w3.org/TR/css-syntax-3/#lowercase-letter
    KRYS_NODISCARD static bool IsLowercaseLetter(char32 codePoint) noexcept
    {
      return Krys::Text::IsASCIILower(codePoint);
    }

    /// @see https://www.w3.org/TR/css-syntax-3/#letter
    KRYS_NODISCARD static bool IsLetter(char32 codePoint) noexcept
    {
      return Krys::Text::IsASCIIAlpha(codePoint);
    }

    /// @see https://www.w3.org/TR/css-syntax-3/#non-ascii-code-point
    KRYS_NODISCARD static bool IsNonASCIICodePoint(char32 codePoint) noexcept
    {
      return codePoint >= 0x80;
    }

    /// @see https://www.w3.org/TR/css-syntax-3/#ident-start-code-point
    KRYS_NODISCARD static bool IsIdentStartCodePoint(char32 codePoint) noexcept
    {
      return IsLetter(codePoint) || IsNonASCIICodePoint(codePoint) || codePoint == U'_';
    }

    /// @see https://www.w3.org/TR/css-syntax-3/#ident-code-point
    KRYS_NODISCARD static bool IsIdentCodePoint(char32 codePoint) noexcept
    {
      return IsIdentStartCodePoint(codePoint) || IsDigit(codePoint) || codePoint == U'-';
    }

    /// @see https://www.w3.org/TR/css-syntax-3/#non-printable-code-point
    KRYS_NODISCARD static bool IsNonPrintableCodePoint(char32 codePoint) noexcept
    {
      return (codePoint >= 0x0 && codePoint <= 0x8) || codePoint == 0xB
             || (codePoint >= 0xE && codePoint <= 0x1F) || codePoint == 0x7F;
    }

    /// @see https://www.w3.org/TR/css-syntax-3/#newline
    KRYS_NODISCARD static bool IsNewline(char32 codePoint) noexcept
    {
      return codePoint == U'\n';
    }

    /// @see https://www.w3.org/TR/css-syntax-3/#whitespace
    KRYS_NODISCARD static bool IsWhitespace(char32 codePoint) noexcept
    {
      return IsNewline(codePoint) || codePoint == U'\t' || codePoint == U' ';
    }

#pragma endregion
  };
}