#include "Krystal.HTML/CSS/Parser/CSSParser.hpp"
#include "Krystal.HTML/CSS/CSSCharsetRule.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSNamespaceRule.hpp"
#include "Krystal.HTML/CSS/CSSRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/CSSStyleRule.hpp"
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
    tokens.DiscardWhitespace();
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

    tokens.DiscardWhitespace();
    if (tokens.IsAtEnd())
    {
      return rule;
    }

    // TODO(CSSParser): parse error (unexpected tokens after rule).
    return nullptr;
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

  RefPtr<CSSRule> CSSParser::ConsumeAtRule(CSSTokenRange &tokens, CSSAllowedRules allowedRules,
                                           bool nested) noexcept
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

  RefPtr<CSSRule> CSSParser::ConsumeQualifiedRule(CSSTokenRange &tokens, CSSAllowedRules allowedRules,
                                                  bool nested) noexcept
  {
    auto prelude = tokens;

    // Parsing a selector (aka a component value) should stop at the first semicolon (and goes to error
    // recovery) instead of consuming the whole list of declarations (in nested context). At top level (aka
    // non nested context), it's the normal rule list error recovery and we don't need this.
    while (!tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::OpenCurly
           && (!nested || tokens.Peek().Type() != CSSTokenType::Semicolon))
    {
      tokens.SkipComponentValue();
    }

    if (tokens.IsAtEnd())
    {
      // TODO(CSSParser): parse error (EOF instead of qualified rule block).
      return nullptr;
    }

    // See comment above
    if (nested && tokens.Peek().Type() == CSSTokenType::Semicolon)
    {
      (void)tokens.Consume();
      return nullptr;
    }

    // TODO(CSSParser): Implement the following rule to disambiguate between a qualified rule and a custom
    // property.
    // https://github.com/w3c/csswg-drafts/issues/9336#issuecomment-1719806755
    // if (range.peek().type() == LeftBraceToken)
    // {
    //   auto rangeCopyForDashedIdent = initialRange;
    //   auto customProperty = CSSPropertyParserHelpers::consumeDashedIdent(rangeCopyForDashedIdent);
    //   // This rule is ambigous with a custom property because it looks like "--ident: ...."
    //   if (customProperty && rangeCopyForDashedIdent.peek().type() == ColonToken)
    //   {
    //     if (isStyleNestedContext())
    //     {
    //       // Error, consume until semicolon or end of block.
    //       while (!range.atEnd() && range.peek().type() != SemicolonToken)
    //         range.consumeComponentValue();
    //       if (range.peek().type() == SemicolonToken)
    //         range.consume();
    //       return {};
    //     }
    //     // Error, consume until end of block.
    //     range.consumeBlock();
    //     return {};
    //   }
    // }

    prelude = prelude.RangeUntil(tokens);
    auto block = tokens.ConsumeBlock();

    if (allowedRules <= CSSAllowedRules::Regular)
    {
      return ConsumeStyleRule(prelude, block);
    }

    // TODO(CSSParser): Implement remaining qualified rule consumers.
    return nullptr;
  }

#pragma endregion

#pragma region At-Rule Consumers

  RefPtr<CSSCharsetRule> CSSParser::ConsumeCharsetRule(CSSTokenRange prelude) noexcept
  {
    const CSSToken &encoding = prelude.Consume();
    prelude.DiscardWhitespace();

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

#pragma endregion

#pragma region Qualified Rule Consumers

  RefPtr<CSSStyleRule> CSSParser::ConsumeStyleRule(CSSTokenRange prelude, CSSTokenRange block) noexcept
  {
    return nullptr;
  }

#pragma endregion
}