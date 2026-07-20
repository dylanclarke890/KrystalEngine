#include "Krystal.HTML/CSS/Parser/CSSParser.hpp"
#include "Krystal.HTML/CSS/CSSCharsetRule.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSNamespaceRule.hpp"
#include "Krystal.HTML/CSS/CSSRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"
#include "Krystal.HTML/CSS/Parser/Enums/CSSAtRuleType.hpp"

namespace Krys::HTML
{
#pragma region Parser Entry Points

  RefPtr<CSSRule> CSSParser::ParseRule(utf32_string &&input, CSSAllowedRules allowedRules) noexcept
  {
    CSSParser parser(Krys::Move(input));

    if (!parser.PumpTokenizer())
    {
      // TODO(CSSParser): parse error (empty rule).
      return nullptr;
    }

    auto tokens = parser.TokenRange();
    tokens.SkipWhitespace();

    if (tokens.IsAtEnd())
    {
      // TODO(CSSParser): parse error (empty rule).
      return nullptr;
    }

    RefPtr<CSSRule> rule = tokens.Peek().Type() == CSSTokenType::AtKeyword
                             ? parser.ConsumeAtRule(tokens, allowedRules)
                             : parser.ConsumeQualifiedRule(tokens, allowedRules);
    if (rule == nullptr)
    {
      return nullptr;
    }

    tokens.SkipWhitespace();
    if (!tokens.IsAtEnd())
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

  RefPtr<CSSRule> CSSParser::ConsumeAtRule(CSSTokenRange &tokens, CSSAllowedRules allowedRules) noexcept
  {
    assert(tokens.Peek().Type() == CSSTokenType::AtKeyword);

    auto name = tokens.Consume().IdentCodePoints();

    auto prelude = tokens;
    while (!tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::OpenCurly
           && tokens.Peek().Type() != CSSTokenType::Semicolon)
    {
      tokens.SkipComponentValue();
    }
    prelude = prelude.RangeUntil(tokens);

    auto atRuleType = ParseCSSAtRuleType(name);
    if (tokens.IsAtEnd() || tokens.Peek().Type() == CSSTokenType::Semicolon)
    {
      (void)tokens.Consume();
      if (atRuleType == CSSAtRuleType::Charset && allowedRules <= CSSAllowedRules::Charset)
      {
        return ConsumeCharsetRule(prelude);
      }
      if (atRuleType == CSSAtRuleType::Import && allowedRules <= CSSAllowedRules::Import)
      {
        return ConsumeImportRule(prelude);
      }
      if (atRuleType == CSSAtRuleType::Namespace && allowedRules <= CSSAllowedRules::Namespace)
      {
        return ConsumeImportRule(prelude);
      }

      // TODO(CSSParser): Implement remaining at-rule consumers for at-rules without blocks.

      // TODO(CSSParser): parse error (unknown at-rule with no block).
      return nullptr;
    }

    CSSTokenRange block = tokens.ConsumeBlock();

    if (atRuleType == CSSAtRuleType::Invalid)
    {
      // TODO(CSSParser): parse error (invalid at-rule).
      return nullptr;
    }

    if (allowedRules == CSSAllowedRules::None)
    {
      return nullptr;
    }

    // TODO(CSSParser): Implement at-rule consumers for at-rules with blocks.

    return nullptr;
  }

  RefPtr<CSSRule> CSSParser::ConsumeQualifiedRule(CSSTokenRange &tokens,
                                                  CSSAllowedRules allowedRules) noexcept
  {
    auto prelude = tokens;
    while (!tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::OpenCurly
           && tokens.Peek().Type() != CSSTokenType::Semicolon)
    {
      tokens.SkipComponentValue();
    }
    prelude = prelude.RangeUntil(tokens);
  }

#pragma endregion

#pragma region At-Rule Consumers

  RefPtr<CSSCharsetRule> CSSParser::ConsumeCharsetRule(CSSTokenRange prelude) noexcept
  {
    const CSSToken &encoding = prelude.Consume();
    prelude.SkipWhitespace();

    if (encoding.Type() != CSSTokenType::String || !prelude.IsAtEnd())
    {
      return nullptr; // Parse error, expected a single string
    }

    return CreateRefPtr<CSSCharsetRule>(nullptr);
  }

  RefPtr<CSSImportRule> CSSParser::ConsumeImportRule(CSSTokenRange prelude) noexcept
  {
    return nullptr;
  }

  RefPtr<CSSNamespaceRule> CSSParser::ConsumeNamespaceRule(CSSTokenRange prelude) noexcept
  {
    return nullptr;
  }

  RefPtr<CSSStyleRule> CSSParser::ConsumeStyleRule(CSSTokenRange prelude) noexcept
  {
    return nullptr;
  }

#pragma endregion
}