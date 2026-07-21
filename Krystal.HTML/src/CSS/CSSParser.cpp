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

  CSSAllowedRules CSSParser::ComputeNextAllowedRules(CSSAllowedRules current, RawPtr<CSSRule> ref) noexcept
  {
    if (!ref || current == CSSAllowedRules::None || current == CSSAllowedRules::Keyframes)
    {
      return current;
    }

    assert(current <= CSSAllowedRules::Regular);

    if (Is<CSSCharsetRule>(ref))
    {
      return CSSAllowedRules::Import;
    }

    if (Is<CSSImportRule>(ref))
    {
      return CSSAllowedRules::Import;
    }

    if (Is<CSSNamespaceRule>(ref))
    {
      return CSSAllowedRules::Namespace;
    }

    return CSSAllowedRules::Regular;
  }

#pragma region Parser Algorithms

  List<Ref<CSSRule>> CSSParser::ConsumeStyleSheetContents(CSSTokenRange &input) noexcept
  {
    CSSAllowedRules allowedRules = CSSAllowedRules::Charset;

    List<Ref<CSSRule>> rules;
    while (true)
    {
      RefPtr<CSSRule> rule;
      switch (input.Peek().Type())
      {
        case CSSTokenType::Whitespace:
        {
          input.Discard();
          continue;
        }
        case CSSTokenType::EndOfFile:
        {
          return rules;
        }
        case CSSTokenType::CDC:
        case CSSTokenType::CDO:
        {
          input.Discard();
          continue;
        }
        case CSSTokenType::AtKeyword:
        {
          rule = ConsumeAtRule(input, allowedRules);
          break;
        }
        default:
        {
          rule = ConsumeQualifiedRule(input, allowedRules);
          break;
        }
      }

      if (rule != nullptr)
      {
        allowedRules = ComputeNextAllowedRules(allowedRules, rule.get());
        rules.push_back(Krys::Move(rule));
      }
    }
  }

  RefPtr<CSSRule> CSSParser::ConsumeAtRule(CSSTokenRange &tokens, CSSAllowedRules allowedRules,
                                           bool nested) noexcept
  {
    assert(tokens.Peek().Type() == CSSTokenType::AtKeyword);

    auto name = tokens.Consume().IdentCodePoints();
    tokens.DiscardWhitespace();

    auto prelude = tokens;
    // we skip the prelude for now, we will parse it later in the specific at-rule consumer
    while (!tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::OpenCurly
           && tokens.Peek().Type() != CSSTokenType::Semicolon)
    {
      tokens.SkipComponentValue();
    }
    prelude = prelude.RangeUntil(tokens);

    auto atRuleType = ParseCSSAtRuleType(name);
    if (tokens.IsAtEnd() || tokens.Peek().Type() == CSSTokenType::Semicolon)
    {
      tokens.Discard(); // consume semicolon if present

      if (atRuleType == CSSAtRuleType::Charset && allowedRules == CSSAllowedRules::Charset)
      {
        return ConsumeCharsetRule(prelude);
      }

      if (atRuleType == CSSAtRuleType::Import && allowedRules <= CSSAllowedRules::Import)
      {
        return ConsumeImportRule(prelude);
      }

      if (atRuleType == CSSAtRuleType::Namespace && allowedRules <= CSSAllowedRules::Namespace)
      {
        return ConsumeNamespaceRule(prelude);
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

    if (allowedRules == CSSAllowedRules::Keyframes)
    {
      // TODO(CSSParser): parse error (at-rule not allowed in @keyframes context).
      return nullptr;
    }

    if (allowedRules == CSSAllowedRules::None)
    {
      // TODO(CSSParser): parse error (rule not allowed in current context).
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
      tokens.Discard();
      return nullptr;
    }

    if (tokens.Peek().Type() == CSSTokenType::OpenCurly)
    {
      auto preludeCopy = prelude;
      preludeCopy.DiscardWhitespace();

      if (preludeCopy.Peek().Type() == CSSTokenType::Ident)
      {
        auto ident = preludeCopy.Consume().IdentCodePoints();
        if (ident.starts_with(u8"--") && preludeCopy.Peek().Type() == CSSTokenType::Colon)
        {
          if (nested)
          {
            ConsumeBadDeclaration(tokens, nested);
            return nullptr;
          }

          (void)tokens.ConsumeBlock();
          return nullptr;
        }
      }
    }

    prelude = prelude.RangeUntil(tokens);
    auto block = tokens.ConsumeBlock();

    if (allowedRules <= CSSAllowedRules::Regular)
    {
      return ConsumeStyleRule(prelude, block);
    }

    // TODO(CSSParser): Implement remaining qualified rule consumers.
    return nullptr;
  }

  void CSSParser::ConsumeBlockContents(CSSTokenRange &tokens, CSSAllowedBlockRules allowedBlockRules) noexcept
  {
  }

  void CSSParser::ConsumeBadDeclaration(CSSTokenRange &tokens, bool nested) noexcept
  {
    while (!tokens.IsAtEnd())
    {
      switch (tokens.Peek().Type())
      {
        case CSSTokenType::Semicolon:
        {
          tokens.Discard();
          return;
        }
        case CSSTokenType::CloseCurly:
        {
          if (nested)
          {
            return;
          }

          tokens.Discard();
          continue;
        }
      }

      tokens.SkipComponentValue();
      continue;
    }
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