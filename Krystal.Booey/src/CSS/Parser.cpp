#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/MediaList.hpp"
#include "Krystal.Booey/CSS/Parser/AtRuleType.hpp"
#include "Krystal.Booey/CSS/Properties/InternalStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParser.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSFunctionDeclarations.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSGroupingRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSNamespaceRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSNestedDeclarations.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRule.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"

namespace krys::boo::css
{
  namespace
  {
    // Check if a CSS rule type does not allow declarations with !important.
    KRYS_NODISCARD bool RuleDoesNotAllowImportant(RuleType type) noexcept
    {
      return type == RuleType::FunctionDeclarations;
    }
  }

#pragma region Parser Entry Points

  Parser::Parser(CSSOMString &&input, const ParserContext &context, StyleSheetContents *stylesheet) noexcept
      : _inputStream(krys::move(input)), _context(context), _tokenizer(_inputStream),
        _ruleListNestingLevel(0uz), _stylesheet(ShareRefPtr(stylesheet))
  {
  }

  RefPtr<CSSStyleSheet> Parser::ParseStylesheet(CSSOMString &&input, const ParserContext &context,
                                                StyleSheetContents &stylesheet) noexcept
  {
    Parser parser(krys::move(input), context, &stylesheet);

    return nullptr;
  }

  RefPtr<CSSRule> Parser::ParseRule(CSSOMString &&input, const ParserContext &context,
                                    AllowedRules allowedRules) noexcept
  {
    Parser parser(krys::move(input), context, nullptr);
    if (!parser.PumpTokenizer())
    {
      // TODO(Parser): parse error (empty rule).
      return nullptr;
    }
    auto tokens = parser.Tokens();

    tokens.DiscardWhitespace();
    if (tokens.IsAtEnd())
    {
      // TODO(Parser): parse error (empty rule).
      return nullptr;
    }

    RefPtr<CSSRule> rule = tokens.Peek().Type() == TokenType::AtKeyword
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

    // TODO(Parser): parse error (unexpected tokens after rule).
    return nullptr;
  }

#pragma endregion

  bool Parser::PumpTokenizer() noexcept
  {
    return _tokenizer.PumpTokenizer();
  }

  TokenRange Parser::Tokens() const noexcept
  {
    return _tokenizer.Tokens();
  }

  AllowedRules Parser::ComputeNextAllowedRules(AllowedRules current, CSSRule *ref) noexcept
  {
    if (!ref || current == AllowedRules::None || current == AllowedRules::Keyframes)
    {
      return current;
    }

    assert(current <= AllowedRules::Regular);

    //if (Is<CSSCharsetRule>(ref))
    //{
    //  return AllowedRules::Import;
    //}

    if (Is<CSSImportRule>(ref))
    {
      return AllowedRules::Import;
    }

    if (Is<CSSNamespaceRule>(ref))
    {
      return AllowedRules::Namespace;
    }

    return AllowedRules::Regular;
  }

  Ref<InternalStyleProperties> Parser::CreateInternalStyleProperties(ParsedPropertyList &properties) noexcept
  {
    auto internalProperties = CreateRef<InternalStyleProperties>();

    // TODO(Parser): Implement the logic to populate internalProperties with the provided properties.

    properties.clear();
    return internalProperties;
  }

#pragma region Parser Algorithms

  List<Ref<CSSRule>> Parser::ConsumeStyleSheetContents(TokenRange &input) noexcept
  {
    AllowedRules allowedRules = AllowedRules::Charset;

    List<Ref<CSSRule>> rules;
    while (true)
    {
      RefPtr<CSSRule> rule;
      switch (input.Peek().Type())
      {
        case TokenType::Whitespace:
        {
          input.Discard();
          continue;
        }
        case TokenType::EndOfFile:
        {
          return rules;
        }
        case TokenType::CDC:
        case TokenType::CDO:
        {
          input.Discard();
          continue;
        }
        case TokenType::AtKeyword:
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
        rules.push_back(krys::move(rule));
      }
    }
  }

  RefPtr<CSSRule> Parser::ConsumeAtRule(TokenRange &tokens, AllowedRules allowedRules, bool nested) noexcept
  {
    assert(tokens.Peek().Type() == TokenType::AtKeyword);

    auto type = tokens.Consume().IdentCodePoints();
    tokens.DiscardWhitespace();

    auto prelude = tokens;
    // we skip the prelude for now, we parse it below in the specific at-rule consumer
    while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::OpenCurly
           && tokens.Peek().Type() != TokenType::Semicolon)
    {
      tokens.DiscardComponentValue();
    }
    prelude = prelude.RangeUntil(tokens);

    auto atRuleType = ParseCSSAtRuleType(type);
    if (tokens.IsAtEnd() || tokens.Peek().Type() == TokenType::Semicolon)
    {
      tokens.Discard(); // consume semicolon if present

      // if (atRuleType == AtRuleType::Charset && allowedRules == AllowedRules::Charset)
      //{
      //   return ConsumeCharsetRule(prelude);
      // }

      if (atRuleType == AtRuleType::Import && allowedRules <= AllowedRules::Import)
      {
        return ConsumeImportRule(prelude);
      }

      if (atRuleType == AtRuleType::Namespace && allowedRules <= AllowedRules::Namespace)
      {
        return ConsumeNamespaceRule(prelude);
      }

      // TODO(Parser): parse error (unknown at-rule with no block).
      return nullptr;
    }

    TokenRange block = tokens.ConsumeBlock();

    if (atRuleType == AtRuleType::Invalid)
    {
      // TODO(Parser): parse error (invalid at-rule).
      return nullptr;
    }

    if (allowedRules == AllowedRules::Keyframes)
    {
      // TODO(Parser): parse error (at-rule not allowed in @keyframes context).
      return nullptr;
    }

    if (allowedRules == AllowedRules::None)
    {
      // TODO(Parser): parse error (rule not allowed in current context).
      return nullptr;
    }

    // TODO(Parser): Implement at-rule consumers for at-rules with blocks.

    return nullptr;
  }

  RefPtr<CSSRule> Parser::ConsumeQualifiedRule(TokenRange &tokens, AllowedRules allowedRules,
                                               bool nested) noexcept
  {
    // Parsing a selector (aka a component value) should stop at the first semicolon (and goes to error
    // recovery) instead of consuming the whole list of declarations (in nested context). At top level (aka
    // non nested context), it's the normal rule list error recovery and we don't need this.
    auto prelude = tokens;
    while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::OpenCurly
           && (!nested || tokens.Peek().Type() != TokenType::Semicolon))
    {
      tokens.DiscardComponentValue();
    }

    if (tokens.IsAtEnd())
    {
      // TODO(Parser): parse error (EOF instead of qualified rule block).
      return nullptr;
    }

    // See comment above
    if (nested && tokens.Peek().Type() == TokenType::Semicolon)
    {
      tokens.Discard();
      return nullptr;
    }

    if (tokens.Peek().Type() == TokenType::OpenCurly)
    {
      auto preludeCopy = prelude;
      preludeCopy.DiscardWhitespace();

      if (preludeCopy.Peek().Type() == TokenType::Ident)
      {
        auto ident = preludeCopy.Consume().IdentCodePoints();
        if (ident.starts_with(u8"--") && preludeCopy.Peek().Type() == TokenType::Colon)
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

    if (allowedRules <= AllowedRules::Regular)
    {
      return ConsumeStyleRule(prelude, block);
    }

    if (allowedRules == AllowedRules::Keyframes)
    {
      return ConsumeKeyframeStyleRule(prelude, block);
    }

    // TODO(Parser): parse error (qualified rule not allowed in current context).
    return nullptr;
  }

  void Parser::ConsumeBlockContents(TokenRange tokens, AllowedBlockRules allowedBlockRules,
                                    RuleType ruleType) noexcept
  {
    assert(allowedBlockRules != AllowedBlockRules::None);
    assert(CurrentNestedContext().ParsedRules.empty());
    assert(CurrentNestedContext().ParsedProperties.empty());

    auto ConsumeUntilSemicolon = [&]() -> void
    {
      while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Semicolon)
      {
        tokens.DiscardComponentValue();
      }
    };

    UniquePtr<ParsedPropertyList> firstDeclarationBlock;
    auto StoreDeclarations = [&]() noexcept -> void
    {
      // We don't wrap the first declaration block, we store it until the end of the style rule.
      // For @function we always use the declaration block.
      if (!firstDeclarationBlock && ruleType != RuleType::FunctionDeclarations)
      {
        firstDeclarationBlock = CreateUnique<ParsedPropertyList>();
        std::swap(*firstDeclarationBlock, CurrentNestedContext().ParsedProperties);
        return;
      }

      // Nothing to wrap
      if (CurrentNestedContext().ParsedProperties.empty())
      {
        return;
      }

      ParsedPropertyList properties;
      std::swap(properties, CurrentNestedContext().ParsedProperties);

      if (ruleType == RuleType::FunctionDeclarations)
      {
        auto rule =
          AdoptRef(*new CSSFunctionDeclarations(CreateInternalStyleProperties(properties), nullptr));
        CurrentNestedContext().ParsedRules.emplace_back(krys::move(rule));
        return;
      }

      auto rule = AdoptRef(*new CSSNestedDeclarations(CreateInternalStyleProperties(properties)));
      CurrentNestedContext().ParsedRules.emplace_back(krys::move(rule));
    };

    while (!tokens.IsAtEnd())
    {
      const auto initialRange = tokens;

      auto ConsumeNestedRuleOrInvalidSyntax = [&]() noexcept -> void
      {
        if (HasFlag(allowedBlockRules, AllowedBlockRules::QualifiedRules))
        {
          assert(IsStyleNestedParsingContext());

          // For block, we try to consume a qualified rule (~= a style rule).
          // This consumes tokens and deals with error recovery in the case of invalid syntax.
          auto rule = ConsumeQualifiedRule(tokens, AllowedRules::Regular);
          if (!Is<CSSStyleRule>(rule.get()))
          {
            return;
          }

          StoreDeclarations();
          CurrentNestedContext().ParsedRules.push_back(krys::move(rule));
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
        case TokenType::Whitespace:
        case TokenType::Semicolon:
        {
          tokens.Discard();
          break;
        }
        case TokenType::Ident:
        {
          if (HasFlag(allowedBlockRules, AllowedBlockRules::Declarations))
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
        case TokenType::AtKeyword:
        {
          if (HasFlag(allowedBlockRules, AllowedBlockRules::AtRules))
          {
            auto allowedRules = ruleType == RuleType::FunctionDeclarations ? AllowedRules::ConditionalGroup
                                                                           : AllowedRules::Regular;

            auto rule = ConsumeAtRule(tokens, allowedRules);
            if (rule == nullptr)
            {
              break;
            }

            auto lastAncestor = CurrentAncestorRuleType();
            assert(lastAncestor);

            // Style rule only support nested group rule.
            if (*lastAncestor == NestedParserContextType::Style && !Is<CSSGroupingRule>(rule.get()))
            {
              break;
            }

            StoreDeclarations();
            CurrentNestedContext().ParsedRules.push_back(krys::move(rule));
          }
          else
          {
            // Rule will be ignored, but consuming the tokens is necessary.
            (void)ConsumeAtRule(tokens, AllowedRules::None);
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

  bool Parser::ConsumeDeclaration(TokenRange &tokens, RuleType ruleType) noexcept
  {
    krys_debug_assert(tokens.Peek().Type() == TokenType::Ident);

    auto &token = tokens.Consume();
    tokens.DiscardWhitespace();

    auto propertyId = FindProperty(token.IdentCodePoints());
    if (tokens.Consume().Type() != TokenType::Colon)
    {
      // TODO(Parser): parse error (expected colon after property name).
      return false;
    }

    tokens.DiscardWhitespace();

    auto important = ConsumeTrailingImportantAndWhitespace(tokens);
    if (important && RuleDoesNotAllowImportant(ruleType))
    {
      return false;
    }

    const size_t oldPropertiesCount = CurrentNestedContext().ParsedProperties.size();
    if (propertyId != PropertyId::Invalid)
    {
      (void)PropertyParser::ParseValue(propertyId, important, tokens, _context,
                                       CurrentNestedContext().ParsedProperties, ruleType);
    }

    return CurrentNestedContext().ParsedProperties.size() != oldPropertiesCount;
  }

  IsImportant Parser::ConsumeTrailingImportantAndWhitespace(TokenRange &tokens)
  {
    tokens.DiscardTrailingWhitespace();
    if (tokens.Size() < 2uz)
    {
      return IsImportant(false);
    }

    auto removeImportantRange = tokens;
    if (auto &last = removeImportantRange.ConsumeLast();
        last.Type() != TokenType::Ident
        || !krys::text::IsASCIICaselessEqual(last.IdentCodePoints(), utf8_stringview(u8"important")))
    {
      return IsImportant(false);
    }

    removeImportantRange.DiscardTrailingWhitespace();
    if (auto &last = removeImportantRange.ConsumeLast();
        last.Type() != TokenType::Delim || last.IdentCodePoints() != u8"!")
    {
      return IsImportant(false);
    }

    removeImportantRange.DiscardTrailingWhitespace();
    tokens = removeImportantRange;
    return IsImportant(true);
  }

  void Parser::ConsumeBadDeclaration(TokenRange &tokens, bool nested) noexcept
  {
    while (!tokens.IsAtEnd())
    {
      switch (tokens.Peek().Type())
      {
        case TokenType::Semicolon:
        {
          tokens.Discard();
          return;
        }
        case TokenType::CloseCurly:
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

  void Parser::ConsumeDeclarationList(TokenRange block, RuleType ruleType) noexcept
  {
    constexpr auto allowed = AllowedBlockRules::Declarations;
    ConsumeBlockContents(block, allowed, ruleType);
  }

  void Parser::ConsumeQualifiedRuleList(TokenRange block, RuleType ruleType) noexcept
  {
    constexpr auto allowed = AllowedBlockRules::QualifiedRules;
    ConsumeBlockContents(block, allowed, ruleType);
  }

  void Parser::ConsumeAtRuleList(TokenRange block, RuleType ruleType) noexcept
  {
    constexpr auto allowed = AllowedBlockRules::AtRules;
    ConsumeBlockContents(block, allowed, ruleType);
  }

  void Parser::ConsumeDeclarationRuleList(TokenRange block, RuleType ruleType) noexcept
  {
    constexpr auto allowed = AllowedBlockRules::Declarations | AllowedBlockRules::AtRules;
    ConsumeBlockContents(block, allowed, ruleType);
  }

  void Parser::ConsumeRuleList(TokenRange block, RuleType ruleType) noexcept
  {
    constexpr auto allowed = AllowedBlockRules::AtRules | AllowedBlockRules::QualifiedRules;
    ConsumeBlockContents(block, allowed, ruleType);
  }

  void Parser::ConsumeStyleBlock(TokenRange block) noexcept
  {
    constexpr auto allowed =
      AllowedBlockRules::Declarations | AllowedBlockRules::QualifiedRules | AllowedBlockRules::AtRules;
    ConsumeBlockContents(block, allowed, RuleType::Style);
  }

#pragma endregion

#pragma region At-Rule Consumers

  // RefPtr<CSSCharsetRule> Parser::ConsumeCharsetRule(TokenRange prelude) noexcept
  //{
  //   // @charset <string> ;

  // const Token &encoding = prelude.Consume();
  // prelude.DiscardWhitespace();

  // if (encoding.Type() != TokenType::String || !prelude.IsAtEnd())
  // {
  //   // TODO(Parser): parse error (expected a single string).
  //   return nullptr;
  // }

  // return AdoptRefPtr<CSSCharsetRule>(new CSSCharsetRule(nullptr));
  // }

  RefPtr<CSSImportRule> Parser::ConsumeImportRule(TokenRange prelude) noexcept
  {
    return nullptr;
  }

  RefPtr<CSSNamespaceRule> Parser::ConsumeNamespaceRule(TokenRange prelude) noexcept
  {
    // @namespace <namespace-prefix>? [ <string> | <url> ] ;
    // <namespace-prefix> = <ident>

    CSSOMStringAtom prefix = CSSOMStringAtom::Null();
    if (prelude.Peek().Type() == TokenType::Ident)
    {
      prefix = prelude.Consume().IdentCodePoints();
      prelude.DiscardWhitespace();
    }

    if (prelude.IsAtEnd())
    {
      // TODO(Parser): parse error (expected a string or url).
      return nullptr;
    }

    if (prelude.Peek().Type() == TokenType::String || prelude.Peek().Type() == TokenType::Url)
    {
      CSSOMStringAtom namespaceURI = prelude.Consume().IdentCodePoints();
      prelude.DiscardWhitespace();

      if (!prelude.IsAtEnd())
      {
        // TODO(Parser): parse error (unexpected tokens after namespace URI).
        return nullptr;
      }

      return CreateRefPtr<CSSNamespaceRule>(prefix, namespaceURI, nullptr);
    }

    return nullptr;
  }

#pragma endregion

#pragma region Qualified Rule Consumers

  RefPtr<CSSStyleRule> Parser::ConsumeStyleRule(TokenRange prelude, TokenRange block) noexcept
  {
    auto mutableSelectors =
      SelectorParser::ParseMutableSelectorList(prelude, {}, _stylesheet.get(), CurrentAncestorRuleType(),
                                               IsForgivingSelectorList(false), DisallowPseudoElements(false));

    if (mutableSelectors.empty())
    {
      return nullptr;
    }

    SelectorList selectors {krys::move(mutableSelectors)};
    assert(!selectors.IsEmpty());

    RefPtr<CSSRule> styleRule;

    RunInNewNestedParsingContext(
      [&]()
      {
        {
          _ancestorRuleTypeStack.push_back(NestedParserContextType::Style);
          ConsumeStyleBlock(block);
          _ancestorRuleTypeStack.pop_back();
        }

        auto &&nestedRules = krys::move(CurrentNestedContext().ParsedRules);
        auto properties = CreateInternalStyleProperties(CurrentNestedContext().ParsedProperties);

        // We save memory by creating a simple CSSStyleRule instead of a heavier CSSNestedDeclarations when we
        // don't need the CSS Nesting features.
        if (nestedRules.empty() && !selectors.HasExplicitNestingParent() && !IsStyleNestedParsingContext())
        {
          styleRule = CreateRefPtr<CSSStyleRule>(krys::move(selectors), krys::move(properties));
        }
        else
        {
          styleRule = CreateRefPtr<CSSNestedDeclarations>(krys::move(selectors), krys::move(properties),
                                                          krys::move(nestedRules));
        }
      });

    return styleRule;
  }

  RefPtr<CSSStyleRule> Parser::ConsumeKeyframeStyleRule(TokenRange prelude, TokenRange block) noexcept
  {
    // TODO:
    return nullptr;
  }

#pragma endregion
}