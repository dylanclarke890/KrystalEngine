#include "Krystal.HTML/CSS/Parser/CSSParser.hpp"
#include "Krystal.HTML/CSS/CSSCharsetRule.hpp"
#include "Krystal.HTML/CSS/CSSFunctionDeclarations.hpp"
#include "Krystal.HTML/CSS/CSSGroupingRule.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSNamespaceRule.hpp"
#include "Krystal.HTML/CSS/CSSNestedDeclarations.hpp"
#include "Krystal.HTML/CSS/CSSRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/CSSStyleProperties.hpp"
#include "Krystal.HTML/CSS/CSSStyleRule.hpp"
#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"
#include "Krystal.HTML/CSS/Parser/Enums/CSSAtRuleType.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParser.hpp"
#include "Krystal.HTML/CSS/StyleSheetContents.hpp"

namespace Krys::HTML
{
  namespace
  {
    // Check if a CSS rule type does not allow declarations with !important.
    KRYS_NODISCARD bool RuleDoesNotAllowImportant(CSSRuleType type) noexcept
    {
      return type == CSSRuleType::FunctionDeclarations;
    }
  }

#pragma region Parser Entry Points

  RefPtr<CSSRule> CSSParser::ParseRule(CSSOMString &&input, CSSAllowedRules allowedRules) noexcept
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

  Ref<CSSInternalStyleProperties>
    CSSParser::CreateInternalStyleProperties(ParsedCSSPropertyList &properties) noexcept
  {
    auto internalProperties = CreateRef<CSSInternalStyleProperties>();

    // TODO(CSSParser): Implement the logic to populate internalProperties with the provided properties.

    properties.clear();
    return internalProperties;
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

    auto type = tokens.Consume().IdentCodePoints();
    tokens.DiscardWhitespace();

    auto prelude = tokens;
    // we skip the prelude for now, we parse it below in the specific at-rule consumer
    while (!tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::OpenCurly
           && tokens.Peek().Type() != CSSTokenType::Semicolon)
    {
      tokens.DiscardComponentValue();
    }
    prelude = prelude.RangeUntil(tokens);

    auto atRuleType = ParseCSSAtRuleType(type);
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
      tokens.DiscardComponentValue();
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

    if (allowedRules == CSSAllowedRules::Keyframes)
    {
      return ConsumeKeyframeStyleRule(prelude, block);
    }

    // TODO(CSSParser): parse error (qualified rule not allowed in current context).
    return nullptr;
  }

  void CSSParser::ConsumeBlockContents(CSSTokenRange tokens, CSSAllowedBlockRules allowedBlockRules,
                                       CSSRuleType ruleType) noexcept
  {
    assert(CurrentNestedContext().ParsedRules.empty());
    assert(CurrentNestedContext().ParsedProperties.empty());

    auto ConsumeUntilSemicolon = [&]() -> void
    {
      while (!tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::Semicolon)
      {
        tokens.DiscardComponentValue();
      }
    };

    UniquePtr<ParsedCSSPropertyList> firstDeclarationBlock;
    auto StoreDeclarations = [&]() noexcept -> void
    {
      // We don't wrap the first declaration block, we store it until the end of the style rule.
      // For @function we always use the declaration block.
      if (!firstDeclarationBlock && ruleType != CSSRuleType::FunctionDeclarations)
      {
        firstDeclarationBlock = CreateUnique<ParsedCSSPropertyList>();
        std::swap(*firstDeclarationBlock, CurrentNestedContext().ParsedProperties);
        return;
      }

      // Nothing to wrap
      if (CurrentNestedContext().ParsedProperties.empty())
      {
        return;
      }

      ParsedCSSPropertyList properties;
      std::swap(properties, CurrentNestedContext().ParsedProperties);

      if (ruleType == CSSRuleType::FunctionDeclarations)
      {
        auto rule =
          AdoptRef(*new CSSFunctionDeclarations(CreateInternalStyleProperties(properties), nullptr));
        CurrentNestedContext().ParsedRules.emplace_back(Krys::Move(rule));
        return;
      }

      auto rule = AdoptRef(*new CSSNestedDeclarations(CreateInternalStyleProperties(properties), nullptr));
      CurrentNestedContext().ParsedRules.emplace_back(Krys::Move(rule));
    };

    while (!tokens.IsAtEnd())
    {
      const auto initialRange = tokens;

      auto ConsumeNestedRuleOrInvalidSyntax = [&]() noexcept -> void
      {
        if (HasFlag(allowedBlockRules, CSSAllowedBlockRules::QualifiedRules))
        {
          assert(IsStyleNestedParsingContext());

          // For block, we try to consume a qualified rule (~= a style rule).
          // This consumes tokens and deals with error recovery
          // in the case of invalid syntax.
          auto rule = ConsumeQualifiedRule(tokens, CSSAllowedRules::Regular);
          if (!Is<CSSStyleRule>(rule.get()))
          {
            return;
          }

          StoreDeclarations();
          CurrentNestedContext().ParsedRules.push_back(Krys::Move(rule));
        }
        else
        {
          // https://drafts.csswg.org/css-syntax/#typedef-declaration-list
          // For declaration list, we consume invalid tokens until next recovery point.
          tokens = initialRange;
          ConsumeUntilSemicolon();
        }
      };

      switch (tokens.Peek().Type())
      {
        case CSSTokenType::Whitespace:
        case CSSTokenType::Semicolon:
        {
          tokens.Discard();
          break;
        }
        case CSSTokenType::Ident:
        {
          if (HasFlag(allowedBlockRules, CSSAllowedBlockRules::Declarations))
          {
            auto declarationStart = tokens;
            ConsumeUntilSemicolon();

            auto declarationRange = declarationStart.RangeUntil(tokens);
            if (!ConsumeDeclaration(declarationRange, ruleType))
            {
              // If it's not a valid declaration, we rewind the parser and try to parse it as a nested style
              // rule.
              tokens = initialRange;
              ConsumeNestedRuleOrInvalidSyntax();
            }
          }
          else
          {
            tokens = initialRange;
            ConsumeNestedRuleOrInvalidSyntax();
          }

          break;
        }
        case CSSTokenType::AtKeyword:
        {
          if (HasFlag(allowedBlockRules, CSSAllowedBlockRules::AtRules))
          {
            auto allowedRules = ruleType == CSSRuleType::FunctionDeclarations
                                  ? CSSAllowedRules::ConditionalGroup
                                  : CSSAllowedRules::Regular;

            auto rule = ConsumeAtRule(tokens, allowedRules);
            if (rule == nullptr)
            {
              break;
            }

            auto lastAncestor = CurrentAncestorRuleType();
            assert(lastAncestor);

            // Style rule only support nested group rule.
            if (*lastAncestor == NestedContextType::Style && !Is<CSSGroupingRule>(rule.get()))
            {
              break;
            }

            StoreDeclarations();
            CurrentNestedContext().ParsedRules.push_back(Krys::Move(rule));
          }
          else
          {
            // Rule will be ignored, but consuming the tokens is necessary.
            (void)ConsumeAtRule(tokens, CSSAllowedRules::None);
          }
          break;
        }
        default:
        {
          ConsumeNestedRuleOrInvalidSyntax();
          break;
        }
      }
    }

    // Store trailing declarations if any
    StoreDeclarations();

    // Restore the initial declaration block
    if (firstDeclarationBlock != nullptr)
    {
      std::swap(*firstDeclarationBlock, CurrentNestedContext().ParsedProperties);
    }
  }

  bool CSSParser::ConsumeDeclaration(CSSTokenRange &tokens, CSSRuleType ruleType) noexcept
  {
    assert(tokens.Peek().Type() == CSSTokenType::Ident);

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    auto propertyId = ParseCSSPropertyId(token.IdentCodePoints());
    if (tokens.Consume().Type() != CSSTokenType::Colon)
    {
      // TODO(CSSParser): parse error (expected colon after property name).
      return false;
    }

    tokens.DiscardWhitespace();

    auto important = ConsumeTrailingImportantAndWhitespace(tokens);
    if (important && RuleDoesNotAllowImportant(ruleType))
    {
      return false;
    }

    const size_t oldPropertiesCount = CurrentNestedContext().ParsedProperties.size();
    if (propertyId != CSSPropertyId::Invalid)
    {
      CSSPropertyParser::ParseValue(tokens, propertyId, ruleType, important,
                                    CurrentNestedContext().ParsedProperties);
    }

    return CurrentNestedContext().ParsedProperties.size() != oldPropertiesCount;
  }

  IsImportant CSSParser::ConsumeTrailingImportantAndWhitespace(CSSTokenRange &range)
  {
    range.DiscardTrailingWhitespace();
    if (range.Size() < 2uz)
    {
      return IsImportant(false);
    }

    auto removeImportantRange = range;
    if (auto &last = removeImportantRange.ConsumeLast();
        last.Type() != CSSTokenType::Ident
        || !Krys::Text::ASCIICaseInsensitiveMatch(last.IdentCodePoints(), u8"important"))
    {
      return IsImportant(false);
    }

    removeImportantRange.DiscardTrailingWhitespace();
    if (auto &last = removeImportantRange.ConsumeLast();
        last.Type() != CSSTokenType::Delim || last.IdentCodePoints() != u8"!")
    {
      return IsImportant(false);
    }

    removeImportantRange.DiscardTrailingWhitespace();
    range = removeImportantRange;
    return IsImportant(true);
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

      tokens.DiscardComponentValue();
      continue;
    }
  }

  void CSSParser::ConsumeDeclarationList(CSSTokenRange block, CSSRuleType ruleType) noexcept
  {
    constexpr static auto allowed = CSSAllowedBlockRules::Declarations;

    ConsumeBlockContents(block, allowed, ruleType);
  }

  void CSSParser::ConsumeQualifiedRuleList(CSSTokenRange block, CSSRuleType ruleType) noexcept
  {
    constexpr static auto allowed = CSSAllowedBlockRules::QualifiedRules;

    ConsumeBlockContents(block, allowed, ruleType);
  }

  void CSSParser::ConsumeAtRuleList(CSSTokenRange block, CSSRuleType ruleType) noexcept
  {
    constexpr static auto allowed = CSSAllowedBlockRules::AtRules;

    ConsumeBlockContents(block, allowed, ruleType);
  }

  void CSSParser::ConsumeDeclarationRuleList(CSSTokenRange block, CSSRuleType ruleType) noexcept
  {
    constexpr static auto allowed = CSSAllowedBlockRules::Declarations | CSSAllowedBlockRules::AtRules;

    ConsumeBlockContents(block, allowed, ruleType);
  }

  void CSSParser::ConsumeRuleList(CSSTokenRange block, CSSRuleType ruleType) noexcept
  {
    constexpr static auto allowed = CSSAllowedBlockRules::AtRules | CSSAllowedBlockRules::QualifiedRules;

    ConsumeBlockContents(block, allowed, ruleType);
  }

  void CSSParser::ConsumeStyleBlock(CSSTokenRange block) noexcept
  {
    constexpr static auto allowed = CSSAllowedBlockRules::Declarations | CSSAllowedBlockRules::QualifiedRules
                                    | CSSAllowedBlockRules::AtRules;

    ConsumeBlockContents(block, allowed, CSSRuleType::Style);
  }

#pragma endregion

#pragma region At-Rule Consumers

  RefPtr<CSSCharsetRule> CSSParser::ConsumeCharsetRule(CSSTokenRange prelude) noexcept
  {
    const CSSToken &encoding = prelude.Consume();
    prelude.DiscardWhitespace();

    if (encoding.Type() != CSSTokenType::String || !prelude.IsAtEnd())
    {
      // TODO(CSSParser): parse error (expected a single string).
      return nullptr;
    }

    return AdoptRefPtr<CSSCharsetRule>(new CSSCharsetRule(nullptr));
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

  RefPtr<CSSStyleRule> CSSParser::ConsumeKeyframeStyleRule(CSSTokenRange prelude,
                                                           CSSTokenRange block) noexcept
  {
    return nullptr;
  }

#pragma endregion
}