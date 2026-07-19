#pragma once

#include "Krystal.HTML/CSS/Parser/CSSInputStream.hpp"
#include "Krystal.HTML/CSS/Parser/CSSToken.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSRule;

  class CSSParser
  {
  private:
    CSSInputStream _inputStream;
    CSSTokenizer _tokenizer;

  public:
    CSSParser(utf32_string &&input) noexcept : _inputStream(Krys::Move(input)), _tokenizer(_inputStream)
    {
    }

#pragma region Parser Entry Points - https://www.w3.org/TR/css-syntax-3/#parser-entry-points

    KRYS_NODISCARD static RefPtr<CSSRule> ParseRule(utf32_string &&input) noexcept;

#pragma endregion

  private:
    /// @brief Pumps the tokenizer until the input stream is exhausted. Returns true if the tokenizer has
    /// consumed at least one token.
    bool PumpTokenizer() noexcept;

    /// @brief Gets the range of tokens produced by the tokenizer.
    /// @note The tokens are only valid as long as the tokenizer is alive. The tokenizer owns the tokens and
    /// will free them when it is destroyed.
    KRYS_NODISCARD CSSTokenRange TokenRange() const noexcept;

#pragma region Parser Algorithms - https://www.w3.org/TR/css-syntax-3/#parser-algorithms

    /// @see https://www.w3.org/TR/css-syntax-3/#consume-an-at-rule
    KRYS_NODISCARD RefPtr<CSSRule> ConsumeAtRule(CSSTokenRange &tokens) noexcept;

    /// @see https://www.w3.org/TR/css-syntax-3/#consume-a-qualified-rule
    KRYS_NODISCARD RefPtr<CSSRule> ConsumeQualifiedRule(CSSTokenRange &tokens) noexcept;

#pragma endregion
  };
}