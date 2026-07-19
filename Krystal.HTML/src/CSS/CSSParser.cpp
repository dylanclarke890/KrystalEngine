#include "Krystal.HTML/CSS/Parser/CSSParser.hpp"
#include "Krystal.HTML/CSS/CSSRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"

namespace Krys::HTML
{
#pragma region Parser Entry Points

  RefPtr<CSSRule> CSSParser::ParseRule(utf32_string &&input) noexcept
  {
    CSSParser parser(Krys::Move(input));

    if (!parser.PumpTokenizer())
    {
      // TODO(CSSParser): parse error (empty rule).
      return nullptr;
    }

    auto tokens = parser.TokenRange();
    tokens.ConsumeWhitespace();

    if (tokens.Peek().Type() == CSSTokenType::EndOfFile)
    {
      // TODO(CSSParser): parse error (empty rule).
      return nullptr;
    }

    RefPtr<CSSRule> rule = tokens.Peek().Type() == CSSTokenType::AtKeyword
                             ? parser.ConsumeAtRule(tokens)
                             : parser.ConsumeQualifiedRule(tokens);
    if (rule == nullptr)
    {
      // TODO(CSSParser): parse error (invalid rule).
      return nullptr;
    }

    tokens.ConsumeWhitespace();
    if (tokens.Peek().Type() == CSSTokenType::EndOfFile)
    {
      // TODO(CSSParser): parse error (unexpected tokens after rule).
      return nullptr;
    }

    return rule;
  }

#pragma endregion

  bool CSSParser::PumpTokenizer() noexcept
  {
    return _tokenizer.PumpTokenizer();
  }

  CSSTokenRange CSSParser::TokenRange() const noexcept
  {
    return _tokenizer.TokenRange();
  }

#pragma region Parser Entry Points

  RefPtr<CSSRule> CSSParser::ConsumeAtRule(CSSTokenRange &tokens) noexcept
  {
    assert(tokens.Peek().Type() == CSSTokenType::AtKeyword);

    auto &atKeywordToken = tokens.Consume();
    auto name = atKeywordToken.IdentCodePoints();

    return nullptr;
  }

  RefPtr<CSSRule> CSSParser::ConsumeQualifiedRule(CSSTokenRange &tokens) noexcept
  {
    return nullptr;
  }

#pragma endregion
}