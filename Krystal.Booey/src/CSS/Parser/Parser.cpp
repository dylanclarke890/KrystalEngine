#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleDescriptors.hpp"
#include "Krystal.Booey/CSS/Parser/CustomPropertySyntax.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserFastPaths.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/SupportsParser.hpp"
#include "Krystal.Booey/CSS/Parser/VariableParser.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Animations.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/CounterStyles.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParser.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Queries/ContainerQueryParser.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Rules/CounterStyleRule.hpp"
#include "Krystal.Booey/CSS/Rules/FunctionRule.hpp"
#include "Krystal.Booey/CSS/Rules/ImportRule.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframesRule.hpp"
#include "Krystal.Booey/CSS/Rules/PositionTryRule.hpp"
#include "Krystal.Booey/CSS/Rules/ViewTransitionRule.hpp"
#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Types/NestingLevelIncrementer.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include <bitset>

namespace krys::boo::css
{
  Parser::~Parser() noexcept = default;

  Parser::Parser(const ParserContext &context, StyleSheetContents *styleSheet) noexcept
      : _context(context), _styleSheet(RefPtr<StyleSheetContents>::WithRef(styleSheet))
  {
  }

  Parser::Parser(const ParserContext &context, CSSOMStringView string, StyleSheetContents *styleSheet,
                 NestedContext nestedContext) noexcept
      : _context(context), _styleSheet(RefPtr<StyleSheetContents>::WithRef(styleSheet))
  {
    InputStream input {CSSOMString(string)};
    _tokenizer = CreateUnique<Tokenizer>(input);
    std::ignore = _tokenizer->PumpTokenizer();

    // With CSSOM, we might want the parser to start in an already nested state.
    if (nestedContext)
    {
      _ancestorRuleTypeStack.push_back(*nestedContext);
    }
  }

  auto Parser::ParseValue(MutableStyleProperties &declaration, PropertyId property, CSSOMStringView string,
                          IsImportant important, const ParserContext &context) noexcept -> ParseResult
  {
    auto ruleType = context.EnclosingRuleType.value_or(RuleType::Style);

    auto state = PropertyParserState {
      .Context = context,
      .CurrentRule = ruleType,
      .CurrentProperty = property,
      .Important = important,
    };
    if (auto value = ParserFastPaths::MaybeParseValue(property, string, state))
    {
      return declaration.AddParsedProperty(Property(property, ShareRef(*value), important))
               ? ParseResult::Changed
               : ParseResult::Unchanged;
    }

    Parser parser(context, string);
    parser.ConsumeDeclarationValue(parser.GetTokenizer()->Tokens(), property, important, ruleType);
    if (parser.TopContext().ParsedProperties.empty())
    {
      return ParseResult::Error;
    }
    return declaration.AddParsedProperties(parser.TopContext().ParsedProperties) ? ParseResult::Changed
                                                                                 : ParseResult::Unchanged;
  }

  auto Parser::ParseCustomPropertyValue(MutableStyleProperties &declaration,
                                        const CSSOMStringAtom &propertyName, CSSOMStringView string,
                                        IsImportant important, const ParserContext &context) noexcept
    -> ParseResult
  {
    Parser parser(context, string);

    auto tokens = parser.GetTokenizer()->Tokens();
    tokens.DiscardWhitespace();
    tokens.DiscardTrailingWhitespace();
    parser.ConsumeCustomPropertyValue(tokens, propertyName, important);

    if (parser.TopContext().ParsedProperties.empty())
    {
      return ParseResult::Error;
    }
    return declaration.AddParsedProperties(parser.TopContext().ParsedProperties) ? ParseResult::Changed
                                                                                 : ParseResult::Unchanged;
  }

  namespace
  {
    using SeenProperties = std::bitset<TotalProperties>;

    // Appends the properties to `output` from the last to the first.
    void FilterProperties(IsImportant important, const ParsedPropertyList &input, ParsedPropertyList &output,
                          SeenProperties &seenProperties,
                          HashSet<CSSOMStringAtom> &seenCustomProperties) noexcept
    {
      // Add properties in reverse order so that highest priority definitions are reached first. Duplicate
      // definitions can then be ignored when found.
      for (size_t i = input.size(); i--;)
      {
        const Property &property = input[i];
        if (property.Important() != important)
        {
          continue;
        }

        if (property.Id() == PropertyId::Custom)
        {
          auto &name = Downcast<CustomPropertyValue>(property.Value()).Name();
          if (!seenCustomProperties.insert(name).second)
          {
            continue;
          }
          output.push_back(property);
          continue;
        }

        auto seenPropertyBit = seenProperties[static_cast<size_t>(property.Id()) - FirstProperty];
        if (seenPropertyBit)
        {
          continue;
        }
        seenPropertyBit = true;

        output.push_back(property);
      }
    }

    // Keeps the last definition of each property, with `!important` ones winning.
    ParsedPropertyList DeduplicateProperties(const ParsedPropertyList &properties) noexcept
    {
      SeenProperties seenProperties;
      ParsedPropertyList results;
      HashSet<CSSOMStringAtom> seenCustomProperties;

      // Filled back to front, as upstream fills a list of the same size from its end.
      FilterProperties(IsImportant(true), properties, results, seenProperties, seenCustomProperties);
      FilterProperties(IsImportant(false), properties, results, seenProperties, seenCustomProperties);
      std::reverse(results.begin(), results.end());

      return results;
    }

    Ref<ImmutableStyleProperties> CreateStyleProperties(ParsedPropertyList &parsedProperties,
                                                        ParserMode mode) noexcept
    {
      auto results = DeduplicateProperties(parsedProperties);

      auto result = ImmutableStyleProperties::CreateDeduplicating(
        Span<const Property>(results.data(), results.size()), mode);
      parsedProperties.clear();
      return result;
    }

    // ASCII whitespace and the vertical tab, as upstream's `deprecatedIsSpaceOrNewline` for ASCII.
    CSSOMString TrimSpaceOrNewline(CSSOMStringView string) noexcept
    {
      while (!string.empty() && text::IsUnicodeCompatibleASCIIWhitespace(string.front()))
      {
        string.remove_prefix(1);
      }
      while (!string.empty() && text::IsUnicodeCompatibleASCIIWhitespace(string.back()))
      {
        string.remove_suffix(1);
      }
      return CSSOMString(string);
    }
  }

  Ref<ImmutableStyleProperties> Parser::ParseInlineStyleDeclaration(CSSOMStringView string,
                                                                    const dom::Element &element) noexcept
  {
    ParserContext context(element.NodeDocument());
    context.Mode =
      StrictToParserMode(Is<html::HTMLElement>(element) && !element.NodeDocument().InQuirksMode());

    Parser parser(context, string);
    parser.ConsumeDeclarationList(parser.GetTokenizer()->Tokens(), RuleType::Style);
    return CreateStyleProperties(parser.TopContext().ParsedProperties, context.Mode);
  }

  bool Parser::ParseDeclarationList(MutableStyleProperties &declaration, CSSOMStringView string,
                                    const ParserContext &context) noexcept
  {
    Parser parser(context, string);
    auto ruleType = context.EnclosingRuleType.value_or(RuleType::Style);
    parser.ConsumeDeclarationList(parser.GetTokenizer()->Tokens(), ruleType);
    if (parser.TopContext().ParsedProperties.empty())
    {
      return false;
    }

    return declaration.AddParsedProperties(DeduplicateProperties(parser.TopContext().ParsedProperties));
  }

  RefPtr<Rule> Parser::ParseRule(CSSOMStringView string, const ParserContext &context,
                                 StyleSheetContents *styleSheet, AllowedRules allowedRules,
                                 NestedContext nestedContext) noexcept
  {
    Parser parser(context, string, styleSheet, nestedContext);
    TokenRange tokens = parser.GetTokenizer()->Tokens();
    tokens.DiscardWhitespace();
    if (tokens.IsAtEnd())
    {
      return nullptr; // Parse error, empty rule
    }
    RefPtr<Rule> rule;
    if (tokens.Peek().Type() == TokenType::AtKeyword)
    {
      rule = parser.ConsumeAtRule(tokens, allowedRules);
    }
    else
    {
      rule = parser.ConsumeQualifiedRule(tokens, allowedRules);
    }
    if (!rule)
    {
      return nullptr; // Parse error, failed to consume rule
    }
    tokens.DiscardWhitespace();
    if (!tokens.IsAtEnd())
    {
      return nullptr; // Parse error, trailing garbage
    }
    return rule;
  }

  RefPtr<KeyframeRule> Parser::ParseKeyframeRule(CSSOMStringView string,
                                                 const ParserContext &context) noexcept
  {
    auto keyframe = ParseRule(string, context, nullptr, AllowedRules::KeyframeRules);
    if (!keyframe)
    {
      return nullptr;
    }
    return RefPtr<KeyframeRule>::WithRef(&Downcast<KeyframeRule>(*keyframe));
  }

  RefPtr<NestedDeclarationsRule> Parser::ParseNestedDeclarations(const ParserContext &context,
                                                                 CSSOMStringView string) noexcept
  {
    auto properties = MutableStyleProperties::CreateEmpty();
    if (!ParseDeclarationList(*properties, string, context))
    {
      return nullptr;
    }

    return NestedDeclarationsRule::Create(krys::move(properties));
  }

  void Parser::ParseStyleSheet(CSSOMStringView string, const ParserContext &context,
                               StyleSheetContents &styleSheet) noexcept
  {
    Parser parser(context, string, &styleSheet);
    bool firstRuleValid =
      parser.ConsumeRuleList(parser.GetTokenizer()->Tokens(), RuleList::TopLevel,
                             [&](Ref<Rule> rule)
                             {
                               if (rule->IsCharsetRule())
                               {
                                 return;
                               }
                               if (context.shouldIgnoreImportRules && rule->IsImportRule())
                               {
                                 return;
                               }
                               styleSheet.ParserAppendRule(krys::move(rule));
                             });
    styleSheet.SetHasSyntacticallyValidCSSHeader(firstRuleValid);
    styleSheet.ShrinkToFit();
  }

  SelectorList Parser::ParsePageSelector(TokenRange tokens, StyleSheetContents *styleSheet) noexcept
  {
    // We only support a small subset of the css-page spec.
    tokens.DiscardWhitespace();
    Maybe<CSSOMStringAtom> typeSelector;
    if (tokens.Peek().Type() == TokenType::Ident)
    {
      typeSelector = CSSOMStringAtom(tokens.Consume().IdentCodePoints());
    }

    Maybe<CSSOMStringView> pseudo;
    if (tokens.Peek().Type() == TokenType::Colon)
    {
      tokens.Discard();
      if (tokens.Peek().Type() != TokenType::Ident)
      {
        return {};
      }
      pseudo = tokens.Consume().IdentCodePoints();
    }

    tokens.DiscardWhitespace();
    if (!tokens.IsAtEnd())
    {
      return {}; // Parse error; extra tokens in @page selector
    }

    // Upstream dereferences the sheet unconditionally; without one, use a sheet's initial default namespace.
    auto defaultNamespace = styleSheet ? styleSheet->DefaultNamespace() : dom::StarAtom();

    UniquePtr<MutableSelector> selector;
    if (typeSelector && !pseudo)
    {
      selector = CreateUnique<MutableSelector>(
        dom::QualifiedName(defaultNamespace, CSSOMStringAtom::Null(), *typeSelector));
    }
    else
    {
      selector = CreateUnique<MutableSelector>();
      if (pseudo)
      {
        selector = MutableSelector::ParsePagePseudoSelector(*pseudo);
        if (!selector || selector->Match() != SelectorMatch::PagePseudoClass)
        {
          return {};
        }
      }
      if (typeSelector)
      {
        selector->AppendTagInComplexSelector(
          dom::QualifiedName(defaultNamespace, CSSOMStringAtom::Null(), *typeSelector));
      }
    }

    selector->SetForPage();
    MutableSelectorList selectors;
    selectors.push_back(krys::move(selector));
    return SelectorList(krys::move(selectors));
  }

  bool Parser::SupportsDeclaration(TokenRange &tokens) noexcept
  {
    bool result = false;

    // We create a new nesting context to isolate the parsing of the @supports(...) prelude from declarations
    // before or after. This only concerns the prelude, (the content of the block will also be in its own
    // nesting context but it's not done here (cf consumeRegularRuleList))
    RunInNewNestingContext(
      [&]
      {
        krys_debug_assert(TopContext().ParsedProperties.empty());
        result = ConsumeDeclaration(tokens, RuleType::Style);
      });

    return result;
  }

  namespace
  {
    Parser::AllowedRules ComputeNewAllowedRules(Parser::AllowedRules allowedRules, Rule *rule) noexcept
    {
      using AllowedRules = Parser::AllowedRules;

      if (!rule || allowedRules == AllowedRules::FontFeatureValuesRules
          || allowedRules == AllowedRules::KeyframeRules || allowedRules == AllowedRules::NoRules)
      {
        return allowedRules;
      }

      krys_debug_assert(allowedRules <= AllowedRules::RegularRules);
      if (rule->IsCharsetRule())
      {
        return AllowedRules::LayerStatementRules;
      }
      if (allowedRules <= AllowedRules::LayerStatementRules && rule->IsLayerRule()
          && Downcast<LayerRule>(*rule).IsStatement())
      {
        return AllowedRules::LayerStatementRules;
      }
      if (rule->IsImportRule())
      {
        return AllowedRules::ImportRules;
      }
      if (rule->IsNamespaceRule())
      {
        return AllowedRules::NamespaceRules;
      }
      return AllowedRules::RegularRules;
    }
  }

  template <typename T>
  bool Parser::ConsumeRuleList(TokenRange tokens, RuleList ruleListType,
                               KRYS_NOESCAPE const T &callback) noexcept
  {
    auto allowedRules = AllowedRules::RegularRules;
    switch (ruleListType)
    {
      case RuleList::TopLevel:          allowedRules = AllowedRules::CharsetRules; break;
      case RuleList::Regular:           allowedRules = AllowedRules::RegularRules; break;
      case RuleList::Keyframes:         allowedRules = AllowedRules::KeyframeRules; break;
      case RuleList::FontFeatureValues: allowedRules = AllowedRules::FontFeatureValuesRules; break;
    }

    bool seenRule = false;
    bool firstRuleValid = false;
    while (!tokens.IsAtEnd())
    {
      RefPtr<Rule> rule;
      switch (tokens.Peek().Type())
      {
        case TokenType::Whitespace: tokens.DiscardWhitespace(); continue;
        case TokenType::AtKeyword:  rule = ConsumeAtRule(tokens, allowedRules); break;
        case TokenType::CDO:
        case TokenType::CDC:
          if (ruleListType == RuleList::TopLevel)
          {
            tokens.Discard();
            continue;
          }
          [[fallthrough]];
        default: rule = ConsumeQualifiedRule(tokens, allowedRules); break;
      }
      if (!seenRule)
      {
        seenRule = true;
        firstRuleValid = rule != nullptr;
      }
      if (rule)
      {
        allowedRules = ComputeNewAllowedRules(allowedRules, rule.get());
        callback(ShareRef(*rule));
      }
    }

    return firstRuleValid;
  }

  RefPtr<Rule> Parser::ConsumeAtRule(TokenRange &tokens, AllowedRules allowedRules) noexcept
  {
    krys_debug_assert(tokens.Peek().Type() == TokenType::AtKeyword);
    const CSSOMStringView name = tokens.ConsumeIncludingWhitespace().IdentCodePoints();
    auto preludeStart = tokens;
    while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::OpenCurly
           && tokens.Peek().Type() != TokenType::Semicolon)
    {
      tokens.DiscardComponentValue();
    }

    auto prelude = preludeStart.RangeUntil(tokens);
    AtRuleId id = AtRuleIdFromName(name);

    if (tokens.IsAtEnd() || tokens.Peek().Type() == TokenType::Semicolon)
    {
      tokens.Discard();
      if (allowedRules == AllowedRules::CharsetRules && id == AtRuleId::Charset)
      {
        return ConsumeCharsetRule(prelude);
      }
      if (allowedRules <= AllowedRules::ImportRules && id == AtRuleId::Import)
      {
        return ConsumeImportRule(prelude);
      }
      if (allowedRules <= AllowedRules::NamespaceRules && id == AtRuleId::Namespace)
      {
        return ConsumeNamespaceRule(prelude);
      }
      if (allowedRules <= AllowedRules::RegularRules && id == AtRuleId::Layer)
      {
        return ConsumeLayerRule(prelude, null);
      }
      return nullptr; // Parse error, unrecognised at-rule without block
    }

    TokenRange block = tokens.ConsumeBlock();
    if (allowedRules == AllowedRules::KeyframeRules)
    {
      return nullptr; // Parse error, no at-rules supported inside @keyframes
    }
    if (allowedRules == AllowedRules::NoRules)
    {
      return nullptr;
    }

    if (allowedRules == AllowedRules::ConditionalGroupRules)
    {
      switch (id)
      {
        case AtRuleId::Media:
        case AtRuleId::Supports:
        case AtRuleId::Container: break;
        case AtRuleId::Function:
          if (!IsFunctionNestedContext())
          {
            return nullptr;
          }
          break;
        default: return nullptr;
      }
    }

    switch (id)
    {
      case AtRuleId::Media:             return ConsumeMediaRule(prelude, block);
      case AtRuleId::Supports:          return ConsumeSupportsRule(prelude, block);
      case AtRuleId::FontFace:          return ConsumeFontFaceRule(prelude, block);
      case AtRuleId::FontFeatureValues: return ConsumeFontFeatureValuesRule(prelude, block);
      case AtRuleId::Styleset:
      case AtRuleId::Stylistic:
      case AtRuleId::CharacterVariant:
      case AtRuleId::Swash:
      case AtRuleId::Ornaments:
      case AtRuleId::Annotation:
        return allowedRules == AllowedRules::FontFeatureValuesRules
                 ? ConsumeFontFeatureValuesRuleBlock(id, prelude, block)
                 : nullptr;
      case AtRuleId::FontPaletteValues: return ConsumeFontPaletteValuesRule(prelude, block);
      case AtRuleId::Keyframes:         return ConsumeKeyframesRule(prelude, block);
      case AtRuleId::Page:              return ConsumePageRule(prelude, block);
      case AtRuleId::CounterStyle:      return ConsumeCounterStyleRule(prelude, block);
      case AtRuleId::Layer:             return ConsumeLayerRule(prelude, block);
      case AtRuleId::Container:         return ConsumeContainerRule(prelude, block);
      case AtRuleId::Property:          return ConsumePropertyRule(prelude, block);
      case AtRuleId::Scope:             return ConsumeScopeRule(prelude, block);
      case AtRuleId::StartingStyle:     return ConsumeStartingStyleRule(prelude, block);
      case AtRuleId::ViewTransition:    return ConsumeViewTransitionRule(prelude, block);
      case AtRuleId::PositionTry:       return ConsumePositionTryRule(prelude, block);
      case AtRuleId::Function:          return ConsumeFunctionRule(prelude, block);
      default:                          return nullptr; // Parse error, unrecognised at-rule with block
    }
  }

  // https://drafts.csswg.org/css-syntax/#consume-a-qualified-rule
  RefPtr<Rule> Parser::ConsumeQualifiedRule(TokenRange &tokens, AllowedRules allowedRules) noexcept
  {
    const auto initialRange = tokens;

    auto isNestedStyleRule = [&]
    {
      return HasStyleRuleAncestor() && allowedRules <= AllowedRules::RegularRules;
    };

    auto preludeStart = tokens;

    // Parsing a selector (aka a component value) should stop at the first semicolon (and goes to error
    // recovery) instead of consuming the whole list of declarations (in nested context). At top level (aka
    // non nested context), it's the normal rule list error recovery and we don't need this.
    while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::OpenCurly
           && (!isNestedStyleRule() || tokens.Peek().Type() != TokenType::Semicolon))
    {
      tokens.DiscardComponentValue();
    }

    if (tokens.IsAtEnd())
    {
      return nullptr; // Parse error, EOF instead of qualified rule block
    }

    // See comment above
    if (isNestedStyleRule() && tokens.Peek().Type() == TokenType::Semicolon)
    {
      tokens.Discard();
      return nullptr;
    }

    // https://github.com/w3c/csswg-drafts/issues/9336#issuecomment-1719806755
    if (tokens.Peek().Type() == TokenType::OpenCurly)
    {
      auto rangeCopyForDashedIdent = initialRange;
      auto customProperty = PropertyParserHelpers::ConsumeDashedIdent(rangeCopyForDashedIdent);
      // This rule is ambigous with a custom property because it looks like "--ident: ...."
      if (customProperty && rangeCopyForDashedIdent.Peek().Type() == TokenType::Colon)
      {
        if (IsStyleNestedContext())
        {
          // Error, consume until semicolon or end of block.
          while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Semicolon)
          {
            tokens.DiscardComponentValue();
          }
          if (tokens.Peek().Type() == TokenType::Semicolon)
          {
            tokens.Discard();
          }
          return nullptr;
        }
        // Error, consume until end of block.
        std::ignore = tokens.ConsumeBlock();
        return nullptr;
      }
    }

    auto prelude = preludeStart.RangeUntil(tokens);
    TokenRange block = ConsumeBlockCheckingForEditability(tokens);

    if (allowedRules <= AllowedRules::RegularRules)
    {
      return ConsumeStyleRule(prelude, block);
    }

    if (allowedRules == AllowedRules::KeyframeRules)
    {
      return ConsumeKeyframeStyleRule(prelude, block);
    }

    return nullptr;
  }

  TokenRange Parser::ConsumeBlockCheckingForEditability(TokenRange &tokens) noexcept
  {
    krys_debug_assert(tokens.Peek().IsBlockStart());

    auto *styleSheet = _styleSheet.get();
    if (styleSheet && !styleSheet->UsesStyleBasedEditability())
    {
      auto blockRange = tokens;
      std::ignore = blockRange.ConsumeBlock();
      for (const auto &token : tokens.RangeUntil(blockRange))
      {
        if (token.Type() == TokenType::Ident
            && text::IsASCIICaselessEqual(token.IdentCodePoints(), utf8_stringview(u8"-webkit-user-modify")))
        {
          styleSheet->ParserSetUsesStyleBasedEditability();
          break;
        }
      }
    }

    return tokens.ConsumeBlock();
  }

  namespace
  {
    // This may still consume tokens if it fails
    Maybe<CSSOMStringAtom> ConsumeStringOrURI(TokenRange &tokens) noexcept
    {
      const Token &token = tokens.Peek();

      if (token.Type() == TokenType::String || token.Type() == TokenType::Url)
      {
        return CSSOMStringAtom(tokens.ConsumeIncludingWhitespace().IdentCodePoints());
      }

      if (token.Type() != TokenType::Function
          || !text::IsASCIICaselessEqual(token.IdentCodePoints(), utf8_stringview(u8"url")))
      {
        return null;
      }

      TokenRange contents = tokens.ConsumeBlock();
      const Token &uri = contents.ConsumeIncludingWhitespace();
      if (uri.Type() == TokenType::BadString || !contents.IsAtEnd())
      {
        return null;
      }
      // Upstream reads the value of whatever token this is; only a string has one.
      if (uri.Type() != TokenType::String)
      {
        return null;
      }
      return CSSOMStringAtom(uri.IdentCodePoints());
    }

    enum class AllowAnonymous : bool
    {
      No,
      Yes
    };

    Maybe<CascadeLayerName> ConsumeCascadeLayerName(TokenRange &tokens,
                                                    AllowAnonymous allowAnonymous) noexcept
    {
      CascadeLayerName name;
      if (tokens.IsAtEnd())
      {
        if (allowAnonymous == AllowAnonymous::Yes)
        {
          return name;
        }
        return null;
      }

      while (true)
      {
        auto &nameToken = tokens.Consume();
        if (nameToken.Type() != TokenType::Ident)
        {
          return null;
        }

        name.push_back(CSSOMStringAtom(nameToken.IdentCodePoints()));

        if (tokens.Peek().Type() != TokenType::Delim || tokens.Peek().Delimiter() != U'.')
        {
          break;
        }
        tokens.Discard();
      }

      tokens.DiscardWhitespace();
      return name;
    }
  }

  RefPtr<CharsetRule> Parser::ConsumeCharsetRule(TokenRange prelude) noexcept
  {
    const Token &string = prelude.ConsumeIncludingWhitespace();
    if (string.Type() != TokenType::String || !prelude.IsAtEnd())
    {
      return nullptr; // Parse error, expected a single string
    }
    return CharsetRule::Create();
  }

  RefPtr<ImportRule> Parser::ConsumeImportRule(TokenRange prelude) noexcept
  {
    auto uri = ConsumeStringOrURI(prelude);
    if (!uri)
    {
      return nullptr; // Parse error, expected string or URI
    }

    prelude.DiscardWhitespace();

    auto consumeCascadeLayer = [&]() -> Maybe<CascadeLayerName>
    {
      auto &token = prelude.Peek();
      if (token.Type() == TokenType::Function
          && text::IsASCIICaselessEqual(token.IdentCodePoints(), utf8_stringview(u8"layer")))
      {
        auto savedPreludeForFailure = prelude;
        auto contents = PropertyParserHelpers::ConsumeFunction(prelude);
        auto layerName = ConsumeCascadeLayerName(contents, AllowAnonymous::No);
        if (!layerName || !contents.IsAtEnd())
        {
          prelude = savedPreludeForFailure;
          return null;
        }
        return layerName;
      }
      if (token.Type() == TokenType::Ident
          && text::IsASCIICaselessEqual(token.IdentCodePoints(), utf8_stringview(u8"layer")))
      {
        std::ignore = prelude.ConsumeIncludingWhitespace();
        return CascadeLayerName {};
      }
      return null;
    };

    auto consumeSupports = [&]() -> Maybe<ImportRule::SupportsCondition>
    {
      auto &token = prelude.Peek();
      if (token.Type() == TokenType::Function
          && text::IsASCIICaselessEqual(token.IdentCodePoints(), utf8_stringview(u8"supports")))
      {
        auto arguments = PropertyParserHelpers::ConsumeFunction(prelude);
        auto supported = SupportsParser::SupportsCondition(
          arguments, *this, SupportsParser::ParsingMode::AllowBareDeclarationAndGeneralEnclosed);
        if (supported == SupportsParser::Invalid)
        {
          return null; // Discard import rule.
        }
        return ImportRule::SupportsCondition {arguments.Serialise(TokenSerialisationMode::Normal),
                                              supported == SupportsParser::Supported};
      }
      return ImportRule::SupportsCondition {};
    };

    auto cascadeLayerName = consumeCascadeLayer();
    auto supports = consumeSupports();
    if (!supports)
    {
      return nullptr; // Discard import rule with incorrect syntax.
    }
    auto mediaQueries = mq::MediaQueryParser::Parse(prelude, _context);

    return ImportRule::Create(CSSOMString(uri->View()), krys::move(mediaQueries),
                              krys::move(cascadeLayerName), krys::move(*supports));
  }

  RefPtr<NamespaceRule> Parser::ConsumeNamespaceRule(TokenRange prelude) noexcept
  {
    auto namespacePrefix = CSSOMStringAtom::Null();
    if (prelude.Peek().Type() == TokenType::Ident)
    {
      namespacePrefix = CSSOMStringAtom(prelude.ConsumeIncludingWhitespace().IdentCodePoints());
    }

    auto uri = ConsumeStringOrURI(prelude);
    if (!uri || !prelude.IsAtEnd())
    {
      return nullptr; // Parse error, expected string or URI
    }

    return NamespaceRule::Create(namespacePrefix, *uri);
  }

  Ref<Rule> Parser::CreateNestedDeclarationsRule() noexcept
  {
    auto properties = CreateStyleProperties(TopContext().ParsedProperties, _context.Mode);
    return NestedDeclarationsRule::Create(krys::move(properties));
  }

  List<Ref<Rule>> Parser::ConsumeNestedGroupRules(TokenRange block) noexcept
  {
    NestingLevelIncrementer incrementer {_ruleListNestingLevel};

    static constexpr auto MaximumRuleListNestingLevel = 128u;
    if (_ruleListNestingLevel > MaximumRuleListNestingLevel)
    {
      return {};
    }

    List<Ref<Rule>> rules;
    // Declarations are allowed if there is either a parent style rule or parent scope rule.
    // https://drafts.csswg.org/css-cascade-6/#scoped-declarations
    if (IsStyleNestedContext())
    {
      RunInNewNestingContext(
        [&]
        {
          ConsumeStyleBlock(block, RuleType::Style);

          if (!TopContext().ParsedProperties.empty())
          {
            // This at-rule contains orphan declarations, we attach them to a nested declaration rule.
            rules.push_back(CreateNestedDeclarationsRule());
          }
          rules.insert(rules.end(), TopContext().ParsedRules.begin(), TopContext().ParsedRules.end());
        });
    }
    else if (IsFunctionNestedContext())
    {
      // Only allow <declaration-rule-list> in @function context.
      auto functionRules = ConsumeDeclarationRuleListInNewNestingContext(block, RuleType::Function);
      rules.insert(rules.end(), functionRules.begin(), functionRules.end());
    }
    else
    {
      ConsumeRuleList(block, RuleList::Regular,
                      [&rules](Ref<Rule> &&rule) { rules.push_back(krys::move(rule)); });
    }
    rules.shrink_to_fit();
    return rules;
  }

  RefPtr<MediaRule> Parser::ConsumeMediaRule(TokenRange prelude, TokenRange block) noexcept
  {
    auto rules = ConsumeNestedGroupRules(block);

    return MediaRule::Create(mq::MediaQueryParser::Parse(prelude, _context), krys::move(rules));
  }

  RefPtr<SupportsRule> Parser::ConsumeSupportsRule(TokenRange prelude, TokenRange block) noexcept
  {
    auto supported =
      SupportsParser::SupportsCondition(prelude, *this, SupportsParser::ParsingMode::ForAtRuleSupports);
    if (supported == SupportsParser::Invalid)
    {
      return nullptr; // Parse error, invalid @supports condition
    }

    auto rules = ConsumeNestedGroupRules(block);

    return SupportsRule::Create(TrimSpaceOrNewline(prelude.Serialise(TokenSerialisationMode::Normal)),
                                supported == SupportsParser::Supported, krys::move(rules));
  }

  RefPtr<FontFaceRule> Parser::ConsumeFontFaceRule(TokenRange prelude, TokenRange block) noexcept
  {
    if (!prelude.IsAtEnd())
    {
      return nullptr; // Parse error; @font-face prelude should be empty
    }

    auto declarations = ConsumeDeclarationListInNewNestingContext(block, RuleType::FontFace);
    return FontFaceRule::Create(CreateStyleProperties(declarations, _context.Mode));
  }

  namespace
  {
    // The associated number represents the maximum number of allowed values for this font-feature-values
    // type. No value means unlimited (for styleset).
    Pair<FontFeatureValuesType, Maybe<uint32>> FontFeatureValuesTypeMappings(AtRuleId id) noexcept
    {
      switch (id)
      {
        case AtRuleId::Styleset:         return {FontFeatureValuesType::Styleset, null};
        case AtRuleId::Stylistic:        return {FontFeatureValuesType::Stylistic, 1u};
        case AtRuleId::CharacterVariant: return {FontFeatureValuesType::CharacterVariant, 2u};
        case AtRuleId::Swash:            return {FontFeatureValuesType::Swash, 1u};
        case AtRuleId::Ornaments:        return {FontFeatureValuesType::Ornaments, 1u};
        case AtRuleId::Annotation:       return {FontFeatureValuesType::Annotation, 1u};
        default:                         krys_debug_assert(false); return {};
      }
    }
  }

  RefPtr<FontFeatureValuesBlockRule>
    Parser::ConsumeFontFeatureValuesRuleBlock(AtRuleId id, TokenRange prelude, TokenRange block) noexcept
  {
    // <feature-value-block> = <font-feature-value-type> { <declaration-list> }
    // <font-feature-value-type> = @stylistic | @historical-forms | @styleset | @character-variant | @swash |
    // @ornaments | @annotation

    // Prelude should be empty.
    if (!prelude.IsAtEnd())
    {
      return nullptr;
    }

    // Block should be present.
    if (block.IsAtEnd())
    {
      return nullptr;
    }

    auto [type, maxValues] = FontFeatureValuesTypeMappings(id);

    auto consumeTag = [this](TokenRange tokens, Maybe<uint32> maxValues) -> Maybe<FontFeatureValuesTag>
    {
      if (tokens.Peek().Type() != TokenType::Ident)
      {
        return null;
      }
      auto name = tokens.ConsumeIncludingWhitespace().IdentCodePoints();
      if (tokens.Consume().Type() != TokenType::Colon)
      {
        return null;
      }
      tokens.DiscardWhitespace();

      auto state = PropertyParserState {.Context = _context};
      List<uint32> values;
      while (!tokens.IsAtEnd())
      {
        auto value = PrimitiveValueResolver<Integer<NonNegative>>::ConsumeAndResolve(tokens, state);
        if (!value)
        {
          return null;
        }
        krys_debug_assert(value->IsInteger());
        auto tagInteger = value->ResolveAsIntegerDeprecated();
        krys_debug_assert(tagInteger >= 0);
        values.push_back(static_cast<uint32>(tagInteger));
        if (maxValues && values.size() > *maxValues)
        {
          return null;
        }
      }
      if (values.empty())
      {
        return null;
      }

      return FontFeatureValuesTag {CSSOMString(name), krys::move(values)};
    };

    List<FontFeatureValuesTag> tags;
    while (!block.IsAtEnd())
    {
      switch (block.Peek().Type())
      {
        case TokenType::Whitespace:
        case TokenType::Semicolon:  block.Discard(); break;
        case TokenType::Ident:
        {
          auto declarationStart = block;

          while (!block.IsAtEnd() && block.Peek().Type() != TokenType::Semicolon)
          {
            block.DiscardComponentValue();
          }

          if (auto tag = consumeTag(declarationStart.RangeUntil(block), maxValues))
          {
            tags.push_back(krys::move(*tag));
          }

          break;
        }
        default: // Parse error, unexpected token in declaration list
          while (!block.IsAtEnd() && block.Peek().Type() != TokenType::Semicolon)
          {
            block.DiscardComponentValue();
          }
          break;
      }
    }
    return FontFeatureValuesBlockRule::Create(type, tags);
  }

  RefPtr<FontFeatureValuesRule> Parser::ConsumeFontFeatureValuesRule(TokenRange prelude,
                                                                     TokenRange block) noexcept
  {
    // @font-feature-values <family-name># { <declaration-list> }

    auto fontFamilies =
      PropertyParserHelpers::ConsumeFontFeatureValuesPreludeFamilyNameList(prelude, _context);
    if (fontFamilies.empty() || !prelude.IsAtEnd())
    {
      return nullptr;
    }

    List<Ref<Rule>> rules;
    ConsumeRuleList(block, RuleList::FontFeatureValues,
                    [&rules](Ref<Rule> &&rule) { rules.push_back(krys::move(rule)); });

    // Convert block rules to value (remove duplicate...etc)
    auto fontFeatureValues = FontFeatureValues::Create();

    for (auto &blockRule : rules)
    {
      if (auto *fontFeatureValuesBlockRule = DynamicDowncast<FontFeatureValuesBlockRule>(blockRule.get()))
      {
        fontFeatureValues->UpdateOrInsertForType(fontFeatureValuesBlockRule->FontFeatureValuesType(),
                                                 fontFeatureValuesBlockRule->Tags());
      }
    }

    return FontFeatureValuesRule::Create(fontFamilies, krys::move(fontFeatureValues));
  }

  RefPtr<FontPaletteValuesRule> Parser::ConsumeFontPaletteValuesRule(TokenRange prelude,
                                                                     TokenRange block) noexcept
  {
    auto name = PropertyParserHelpers::ConsumeDashedIdent(prelude);
    if (!name || !prelude.IsAtEnd())
    {
      return nullptr; // Parse error; expected custom ident in @font-palette-values header
    }

    auto declarations = ConsumeDeclarationListInNewNestingContext(block, RuleType::FontPaletteValues);
    auto properties = CreateStyleProperties(declarations, _context.Mode);

    auto fontFamilies = [&]
    {
      List<CSSOMStringAtom> fontFamilies;
      auto append = [&](const PrimitiveValue &value)
      {
        if (value.IsFontFamily())
        {
          fontFamilies.push_back(CSSOMStringAtom(value.StringValue()));
        }
      };
      auto cssFontFamily = properties->GetPropertyCSSValue(PropertyId::FontFamily);
      if (!cssFontFamily)
      {
        return fontFamilies;
      }
      if (auto *families = DynamicDowncast<ValueList>(*cssFontFamily))
      {
        for (auto &item : *families)
        {
          append(Downcast<PrimitiveValue>(item));
        }
        return fontFamilies;
      }
      if (auto *family = DynamicDowncast<PrimitiveValue>(*cssFontFamily))
      {
        append(*family);
      }
      return fontFamilies;
    }();

    Maybe<FontPaletteIndex> basePalette;
    if (auto basePaletteValue = properties->GetPropertyCSSValue(PropertyId::BasePalette))
    {
      const auto &primitiveValue = Downcast<PrimitiveValue>(*basePaletteValue);
      if (primitiveValue.IsInteger())
      {
        basePalette = FontPaletteIndex(primitiveValue.ResolveAsIntegerDeprecated<uint32>());
      }
      else if (primitiveValue.ValueId() == ValueId::Light)
      {
        basePalette = FontPaletteIndex(FontPaletteIndex::Type::Light);
      }
      else if (primitiveValue.ValueId() == ValueId::Dark)
      {
        basePalette = FontPaletteIndex(FontPaletteIndex::Type::Dark);
      }
    }

    List<FontPaletteValues::OverriddenColor> overrideColors;
    if (auto overrideColorsValue = properties->GetPropertyCSSValue(PropertyId::OverrideColors))
    {
      for (auto &item : Downcast<ValueList>(*overrideColorsValue))
      {
        const auto &pair = Downcast<ValuePair>(item);

        auto key = Downcast<PrimitiveValue>(pair.First()).ResolveAsIntegerDeprecated<uint32>();
        auto color = ColorValue::GetAbsoluteColor(pair.Second());
        if (!color.IsValid())
        {
          continue;
        }

        overrideColors.push_back({key, krys::move(color)});
      }
    }

    return FontPaletteValuesRule::Create(CSSOMStringAtom(name->StringValue()), krys::move(fontFamilies),
                                         krys::move(basePalette), krys::move(overrideColors));
  }

  RefPtr<KeyframesRule> Parser::ConsumeKeyframesRule(TokenRange prelude, TokenRange block) noexcept
  {
    const Token &nameToken = prelude.ConsumeIncludingWhitespace();
    if (!prelude.IsAtEnd())
    {
      return nullptr; // Parse error; expected single non-whitespace token in @keyframes header
    }

    if (nameToken.Type() == TokenType::Ident)
    {
      // According to the CSS Values specification, identifier-based keyframe names
      // are not allowed to be CSS wide keywords or "default". And CSS Animations
      // additionally excludes the "none" keyword.
      if (!IsValidCustomIdentifier(nameToken.ValueId()) || nameToken.ValueId() == ValueId::None)
      {
        return nullptr;
      }
    }
    else if (nameToken.Type() != TokenType::String)
    {
      return nullptr; // Parse error; expected ident token or string in @keyframes header
    }

    auto name = CSSOMStringAtom(nameToken.IdentCodePoints());

    auto keyframeRule = KeyframesRule::Create(name);
    ConsumeRuleList(block, RuleList::Keyframes,
                    [&keyframeRule](Ref<Rule> keyframe)
                    {
                      keyframeRule->ParserAppendKeyframe(
                        RefPtr<KeyframeRule>::WithRef(&Downcast<KeyframeRule>(*keyframe)));
                    });

    keyframeRule->ShrinkToFit();
    return keyframeRule;
  }

  RefPtr<PageRule> Parser::ConsumePageRule(TokenRange prelude, TokenRange block) noexcept
  {
    auto selectorList = ParsePageSelector(prelude, _styleSheet.get());
    if (selectorList.IsEmpty())
    {
      return nullptr; // Parse error, invalid @page selector
    }

    auto declarations = ConsumeDeclarationListInNewNestingContext(block, RuleType::Page);

    return PageRule::Create(CreateStyleProperties(declarations, _context.Mode), krys::move(selectorList));
  }

  RefPtr<CounterStyleRule> Parser::ConsumeCounterStyleRule(TokenRange prelude, TokenRange block) noexcept
  {
    auto rangeCopy = prelude;
    auto name = PropertyParserHelpers::ConsumeCounterStyleNameInPrelude(rangeCopy, _context.Mode);
    if (name == CSSOMStringAtom::Null())
    {
      return nullptr;
    }

    auto declarations = ConsumeDeclarationListInNewNestingContext(block, RuleType::CounterStyle);
    auto descriptors =
      CounterStyleDescriptors::Create(name, *CreateStyleProperties(declarations, _context.Mode));
    if (!descriptors.IsValid())
    {
      return nullptr;
    }
    return CounterStyleRule::Create(name, krys::move(descriptors));
  }

  RefPtr<ViewTransitionRule> Parser::ConsumeViewTransitionRule(TokenRange prelude, TokenRange block) noexcept
  {
    if (!prelude.IsAtEnd())
    {
      return nullptr; // Parse error; @view-transition prelude should be empty
    }

    auto declarations = ConsumeDeclarationListInNewNestingContext(block, RuleType::ViewTransition);
    return ViewTransitionRule::Create(CreateStyleProperties(declarations, _context.Mode));
  }

  RefPtr<PositionTryRule> Parser::ConsumePositionTryRule(TokenRange prelude, TokenRange block) noexcept
  {
    // Prelude should ONLY be a <dashed-ident>.
    auto ruleName = PropertyParserHelpers::ConsumeDashedIdentRaw(prelude);
    if (!ruleName)
    {
      return nullptr;
    }
    if (!prelude.IsAtEnd())
    {
      return nullptr;
    }

    auto declarations = ConsumeDeclarationListInNewNestingContext(block, RuleType::PositionTry);
    return PositionTryRule::Create(CSSOMStringAtom(*ruleName),
                                   CreateStyleProperties(declarations, _context.Mode));
  }

  RefPtr<FunctionRule> Parser::ConsumeFunctionRule(TokenRange prelude, TokenRange block) noexcept
  {
    // https://drafts.csswg.org/css-mixins/#function-rule
    // <@function> = @function <function-token> <function-parameter>#? ) [ returns <css-type> ]?

    if (prelude.Peek().Type() != TokenType::Function)
    {
      return nullptr;
    }

    auto name = CSSOMStringAtom(prelude.Peek().IdentCodePoints());
    auto parametersRange = PropertyParserHelpers::ConsumeFunction(prelude);

    // <function-parameter>#?
    List<FunctionRule::Parameter> parameters;
    while (!parametersRange.IsAtEnd())
    {
      auto consumeParameter = [&]() -> Maybe<FunctionRule::Parameter>
      {
        // <function-parameter> = <custom-property-name> <css-type>? [ : <default-value> ]?

        auto &nameToken = parametersRange.ConsumeIncludingWhitespace();
        if (nameToken.Type() != TokenType::Ident)
        {
          return null;
        }

        // <custom-property-name>
        auto parameter = FunctionRule::Parameter {CSSOMStringAtom(nameToken.IdentCodePoints()),
                                                  CustomPropertySyntax::Universal(), nullptr};
        if (!IsCustomPropertyName(parameter.Name.View()))
        {
          return null;
        }

        if (parametersRange.IsAtEnd() || parametersRange.Peek().Type() == TokenType::Comma)
        {
          return parameter;
        }

        // <css-type>?
        if (parametersRange.Peek().Type() != TokenType::Colon)
        {
          auto type = CustomPropertySyntax::ConsumeType(parametersRange);
          if (!type)
          {
            return null;
          }
          parameter.Type = *type;
        }

        // [ : <default-value> ]?
        if (parametersRange.Peek().Type() == TokenType::Colon)
        {
          std::ignore = parametersRange.ConsumeIncludingWhitespace();
          // <default-value> = <declaration-value>
          auto defaultRangeStart = parametersRange;
          while (!parametersRange.IsAtEnd() && parametersRange.Peek().Type() != TokenType::Comma)
          {
            if (parametersRange.Peek().Type() == TokenType::Delim
                && parametersRange.Peek().Delimiter() == U'!')
            {
              return null;
            }
            std::ignore = parametersRange.ConsumeIncludingWhitespace();
          }

          auto defaultRange = defaultRangeStart.RangeUntil(parametersRange);

          // "If a default value and a parameter type are both provided, then the default value must parse
          // successfully according to that parameter type's syntax. Otherwise, the @function rule is
          // invalid."
          if (!PropertyParser::IsValidCustomPropertyValueForSyntax(parameter.Type, defaultRange, _context))
          {
            return null;
          }

          parameter.DefaultValue = VariableData::Create(defaultRange);
        }

        if (parametersRange.IsAtEnd() || parametersRange.Peek().Type() == TokenType::Comma)
        {
          return parameter;
        }

        return null;
      };

      auto parameter = consumeParameter();
      if (!parameter)
      {
        return nullptr;
      }
      parameters.push_back(krys::move(*parameter));

      if (parametersRange.Peek().Type() == TokenType::Comma)
      {
        std::ignore = parametersRange.ConsumeIncludingWhitespace();
      }
    }

    auto returnType = CustomPropertySyntax::Universal();

    // [ returns <css-type> ]?
    if (prelude.Peek().Type() == TokenType::Ident
        && text::IsASCIICaselessEqual(prelude.Peek().IdentCodePoints(), utf8_stringview(u8"returns")))
    {
      std::ignore = prelude.ConsumeIncludingWhitespace();

      auto specifiedReturnType = CustomPropertySyntax::ConsumeType(prelude);
      if (!specifiedReturnType)
      {
        return nullptr;
      }
      returnType = *specifiedReturnType;
    }

    if (!prelude.IsAtEnd())
    {
      return nullptr;
    }

    _ancestorRuleTypeStack.push_back(NestedParserContextType::Function);
    auto functionBody = ConsumeDeclarationRuleListInNewNestingContext(block, RuleType::Function);
    _ancestorRuleTypeStack.pop_back();

    return FunctionRule::Create(name, krys::move(parameters), krys::move(returnType),
                                krys::move(functionBody));
  }

  RefPtr<ScopeRule> Parser::ConsumeScopeRule(TokenRange prelude, TokenRange block) noexcept
  {
    SelectorList scopeStart;
    SelectorList scopeEnd;

    if (!prelude.IsAtEnd())
    {
      auto consumePrelude = [&]
      {
        auto consumeScope = [&](SelectorList &scope, NestedContext ancestorRuleType)
        {
          // Consume the left parenthesis
          if (prelude.Peek().Type() != TokenType::OpenParen)
          {
            return false;
          }
          std::ignore = prelude.ConsumeIncludingWhitespace();

          // Determine the range for the selector list
          auto selectorListRangeStart = prelude;
          while (!prelude.IsAtEnd() && prelude.Peek().Type() != TokenType::CloseParen)
          {
            prelude.DiscardComponentValue();
          }
          auto selectorListRange = selectorListRangeStart.RangeUntil(prelude);

          // Parse the selector list range
          auto mutableSelectorList = ParseMutableSelectorList(
            selectorListRange, SelectorParserContext(_context), _styleSheet.get(), ancestorRuleType,
            IsForgivingSelectorList(false), DisallowPseudoElements(true));
          if (mutableSelectorList.empty())
          {
            return false;
          }

          // Consume the right parenthesis
          if (prelude.Peek().Type() != TokenType::CloseParen)
          {
            return false;
          }
          std::ignore = prelude.ConsumeIncludingWhitespace();

          // Return the correctly parsed scope
          scope = SelectorList(krys::move(mutableSelectorList));
          return true;
        };
        auto successScopeStart = consumeScope(scopeStart, LastAncestorRuleType());
        if (successScopeStart && prelude.IsAtEnd())
        {
          return true;
        }
        if (prelude.Peek().Type() != TokenType::Ident)
        {
          return false;
        }
        auto &to = prelude.ConsumeIncludingWhitespace();
        if (!text::IsASCIICaselessEqual(to.IdentCodePoints(), utf8_stringview(u8"to")))
        {
          return false;
        }
        // scopeEnd is always considered nested, at least by the scopeStart
        if (!consumeScope(scopeEnd, NestedParserContextType::Scope))
        {
          return false;
        }
        if (!prelude.IsAtEnd())
        {
          return false;
        }
        return true;
      };
      if (!consumePrelude())
      {
        return nullptr;
      }
    }

    _ancestorRuleTypeStack.push_back(NestedParserContextType::Scope);
    auto rules = ConsumeNestedGroupRules(block);
    _ancestorRuleTypeStack.pop_back();
    auto rule = ScopeRule::Create(krys::move(scopeStart), krys::move(scopeEnd), krys::move(rules));
    if (_styleSheet)
    {
      rule->SetStyleSheetContents(*_styleSheet);
    }
    return rule;
  }

  RefPtr<StartingStyleRule> Parser::ConsumeStartingStyleRule(TokenRange prelude, TokenRange block) noexcept
  {
    if (!prelude.IsAtEnd())
    {
      return nullptr;
    }

    auto rules = ConsumeNestedGroupRules(block);

    return StartingStyleRule::Create(krys::move(rules));
  }

  RefPtr<LayerRule> Parser::ConsumeLayerRule(TokenRange prelude, Maybe<TokenRange> block) noexcept
  {
    if (!block)
    {
      // List syntax.
      List<CascadeLayerName> nameList;
      while (true)
      {
        auto name = ConsumeCascadeLayerName(prelude, AllowAnonymous::No);
        if (!name)
        {
          return nullptr;
        }
        nameList.push_back(krys::move(*name));

        if (prelude.IsAtEnd())
        {
          break;
        }

        auto &commaToken = prelude.ConsumeIncludingWhitespace();
        if (commaToken.Type() != TokenType::Comma)
        {
          return nullptr;
        }
      }

      return LayerRule::CreateStatement(krys::move(nameList));
    }

    auto name = ConsumeCascadeLayerName(prelude, AllowAnonymous::Yes);
    if (!name)
    {
      return nullptr;
    }

    // No comma separated list when using the block syntax.
    if (!prelude.IsAtEnd())
    {
      return nullptr;
    }

    auto rules = ConsumeNestedGroupRules(*block);

    return LayerRule::CreateBlock(krys::move(*name), krys::move(rules));
  }

  RefPtr<ContainerRule> Parser::ConsumeContainerRule(TokenRange prelude, TokenRange block) noexcept
  {
    if (prelude.IsAtEnd())
    {
      return nullptr;
    }

    auto query = cq::ContainerQueryParser::ConsumeContainerQuery(prelude, _context);
    if (!query)
    {
      return nullptr;
    }

    prelude.DiscardWhitespace();
    if (!prelude.IsAtEnd())
    {
      return nullptr;
    }

    auto rules = ConsumeNestedGroupRules(block);

    return ContainerRule::Create(krys::move(*query), krys::move(rules));
  }

  RefPtr<PropertyRule> Parser::ConsumePropertyRule(TokenRange prelude, TokenRange block) noexcept
  {
    auto &nameToken = prelude.ConsumeIncludingWhitespace();
    if (nameToken.Type() != TokenType::Ident || !prelude.IsAtEnd())
    {
      return nullptr;
    }

    auto name = CSSOMStringAtom(nameToken.IdentCodePoints());
    if (!IsCustomPropertyName(name.View()))
    {
      return nullptr;
    }

    auto declarations = ConsumeDeclarationListInNewNestingContext(block, RuleType::Property);

    auto descriptor = PropertyRule::Descriptor {name};
    // Upstream's syntax string is null until set.
    bool hasSyntax = false;

    for (auto &property : declarations)
    {
      switch (property.Id())
      {
        case PropertyId::Syntax:
          descriptor.Syntax = Downcast<PrimitiveValue>(property.Value()).StringValue();
          hasSyntax = true;
          continue;
        case PropertyId::Inherits: descriptor.Inherits = property.Value().ValueId() == ValueId::True; break;
        case PropertyId::InitialValue:
          descriptor.InitialValue = Downcast<CustomPropertyValue>(property.Value()).AsVariableData();
          break;
        default: break;
      }
    }

    // "The inherits descriptor is required for the @property rule to be valid; if it's missing, the @property
    // rule is invalid." https://drafts.css-houdini.org/css-properties-values-api/#inherits-descriptor
    if (!descriptor.Inherits)
    {
      return nullptr;
    }

    // "If the provided string is not a valid syntax string, the descriptor is invalid and must be ignored."
    // https://drafts.css-houdini.org/css-properties-values-api/#the-syntax-descriptor
    if (!hasSyntax)
    {
      return nullptr;
    }
    auto syntax = CustomPropertySyntax::Parse(descriptor.Syntax);
    if (!syntax)
    {
      return nullptr;
    }

    // "The initial-value descriptor is optional only if the syntax is the universal syntax definition,
    // otherwise the descriptor is required; if it's missing, the entire rule is invalid and must be ignored."
    if (!syntax->IsUniversal())
    {
      if (!descriptor.InitialValue)
      {
        return nullptr;
      }
    }

    auto initialValueIsValid = [&]
    {
      auto tokens = descriptor.InitialValue->TokenRange();
      auto dependencies =
        PropertyParser::CollectParsedCustomPropertyValueDependencies(*syntax, tokens, _context);
      if (!dependencies.IsComputationallyIndependent())
      {
        return false;
      }
      auto containsVariable =
        VariableParser::ContainsValidVariableReferences(descriptor.InitialValue->TokenRange(), _context);
      if (containsVariable)
      {
        return false;
      }
      return true;
    };
    if (descriptor.InitialValue && !initialValueIsValid())
    {
      return nullptr;
    }

    return PropertyRule::Create(krys::move(descriptor));
  }

  RefPtr<KeyframeRule> Parser::ConsumeKeyframeStyleRule(TokenRange prelude, TokenRange block) noexcept
  {
    auto state = PropertyParserState {.Context = _context};
    auto keyList = PropertyParserHelpers::ConsumeKeyframeKeyList(prelude, state);
    if (keyList.empty())
    {
      return nullptr;
    }

    auto declarations = ConsumeDeclarationListInNewNestingContext(block, RuleType::Keyframe);

    return KeyframeRule::Create(Span<const Pair<ValueId, double>>(keyList.data(), keyList.size()),
                                CreateStyleProperties(declarations, _context.Mode));
  }

  RefPtr<Rule> Parser::ConsumeStyleRule(TokenRange prelude, TokenRange block) noexcept
  {
    auto mutableSelectorList = ParseMutableSelectorList(
      prelude, SelectorParserContext(_context), _styleSheet.get(), LastAncestorRuleType(),
      IsForgivingSelectorList(false), DisallowPseudoElements(false));

    if (mutableSelectorList.empty())
    {
      return nullptr; // Parse error, invalid selector list
    }

    SelectorList selectorList(krys::move(mutableSelectorList));
    krys_debug_assert(!selectorList.IsEmpty());

    RefPtr<Rule> styleRule;

    RunInNewNestingContext(
      [&]
      {
        {
          _ancestorRuleTypeStack.push_back(NestedParserContextType::Style);
          ConsumeStyleBlock(block, RuleType::Style);
          _ancestorRuleTypeStack.pop_back();
        }

        auto nestedRules = krys::move(TopContext().ParsedRules);
        auto properties = CreateStyleProperties(TopContext().ParsedProperties, _context.Mode);

        // We save memory by creating a simple StyleRule instead of a heavier StyleRuleWithNesting when we
        // don't need the CSS Nesting features.
        if (nestedRules.empty() && !selectorList.HasExplicitNestingParent() && !IsStyleNestedContext())
        {
          styleRule = StyleRule::Create(krys::move(properties), _context.HasDocumentSecurityOrigin,
                                        krys::move(selectorList));
        }
        else
        {
          styleRule = StyleRuleWithNesting::Create(krys::move(properties), _context.HasDocumentSecurityOrigin,
                                                   krys::move(selectorList), krys::move(nestedRules));
        }
      });

    return styleRule;
  }

  // https://drafts.csswg.org/css-syntax/#consume-block-contents
  // https://drafts.csswg.org/css-syntax/#block-contents
  void Parser::ConsumeBlockContent(TokenRange tokens, RuleType ruleType,
                                   EnumSet<BlockAllowedRule> blockAllowedRules) noexcept
  {
    krys_debug_assert(TopContext().ParsedProperties.empty());
    krys_debug_assert(TopContext().ParsedRules.empty());

    // All the current callers support declarations so the no-declarations case is not implemented.
    krys_debug_assert(blockAllowedRules.Contains(BlockAllowedRule::Declarations));

    auto consumeUntilSemicolon = [&]
    {
      while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Semicolon)
      {
        tokens.DiscardComponentValue();
      }
    };

    UniquePtr<ParsedPropertyList> initialDeclarationBlock;
    auto storeDeclarations = [&]
    {
      // We don't wrap the first declaration block, we store it until the end of the style rule.
      // For @function we always use the declaration block.
      if (!initialDeclarationBlock && ruleType != RuleType::Function)
      {
        initialDeclarationBlock = CreateUnique<ParsedPropertyList>();
        std::swap(*initialDeclarationBlock, TopContext().ParsedProperties);
        return;
      }

      // Nothing to wrap
      if (TopContext().ParsedProperties.empty())
      {
        return;
      }

      ParsedPropertyList properties;
      std::swap(properties, TopContext().ParsedProperties);

      if (ruleType == RuleType::Function)
      {
        auto rule = FunctionDeclarationsRule::Create(CreateStyleProperties(properties, _context.Mode));
        TopContext().ParsedRules.push_back(krys::move(rule));
        return;
      }

      auto rule = NestedDeclarationsRule::Create(CreateStyleProperties(properties, _context.Mode));
      TopContext().ParsedRules.push_back(krys::move(rule));
    };

    while (!tokens.IsAtEnd())
    {
      const auto initialRange = tokens;

      auto consumeNestedRuleOrInvalidSyntax = [&]
      {
        if (blockAllowedRules.Contains(BlockAllowedRule::QualifiedRules))
        {
          krys_debug_assert(IsStyleNestedContext());
          // For block, we try to consume a qualified rule (~= a style rule).
          // This consumes tokens and deals with error recovery
          // in the case of invalid syntax.
          auto rule = ConsumeQualifiedRule(tokens, AllowedRules::RegularRules);
          if (!rule)
          {
            return;
          }
          if (!rule->IsStyleRule())
          {
            return;
          }
          storeDeclarations();
          TopContext().ParsedRules.push_back(ShareRef(*rule));
        }
        else
        {
          // https://drafts.csswg.org/css-syntax/#typedef-declaration-list
          // For declaration list, we consume invalid tokens until next recovery point.
          tokens = initialRange;
          consumeUntilSemicolon();
        }
      };

      switch (tokens.Peek().Type())
      {
        case TokenType::Whitespace:
        case TokenType::Semicolon:  tokens.Discard(); break;
        case TokenType::Ident:
        {
          auto declarationStart = tokens;

          consumeUntilSemicolon();

          auto declarationRange = declarationStart.RangeUntil(tokens);
          auto isValidDeclaration = ConsumeDeclaration(declarationRange, ruleType);

          if (!isValidDeclaration)
          {
            // If it's not a valid declaration, we rewind the parser and try to parse it as a nested style
            // rule.
            tokens = initialRange;
            consumeNestedRuleOrInvalidSyntax();
          }
          break;
        }
        case TokenType::AtKeyword:
        {
          if (blockAllowedRules.Contains(BlockAllowedRule::AtRules))
          {
            auto allowedRules = ruleType == RuleType::Function ? AllowedRules::ConditionalGroupRules
                                                               : AllowedRules::RegularRules;
            auto rule = ConsumeAtRule(tokens, allowedRules);
            if (!rule)
            {
              break;
            }
            auto lastAncestor = LastAncestorRuleType();
            krys_debug_assert(lastAncestor.has_value());
            // Style rule only support nested group rule.
            if (lastAncestor == NestedParserContextType::Style && !rule->IsGroupRule())
            {
              break;
            }
            storeDeclarations();
            TopContext().ParsedRules.push_back(ShareRef(*rule));
          }
          else
          {
            // Rule will be ignored, but consuming the tokens is necessary.
            [[maybe_unused]] auto rule = ConsumeAtRule(tokens, AllowedRules::NoRules);
            krys_debug_assert(!rule);
          }
          break;
        }
        default: consumeNestedRuleOrInvalidSyntax();
      }
    }

    // Store trailing declarations if any
    storeDeclarations();

    // Restore the initial declaration block
    if (initialDeclarationBlock)
    {
      std::swap(*initialDeclarationBlock, TopContext().ParsedProperties);
    }
  }

  ParsedPropertyList Parser::ConsumeDeclarationListInNewNestingContext(TokenRange tokens,
                                                                       RuleType ruleType) noexcept
  {
    ParsedPropertyList result;
    RunInNewNestingContext(
      [&]
      {
        ConsumeDeclarationList(tokens, ruleType);
        result = krys::move(TopContext().ParsedProperties);
      });
    return result;
  }

  List<Ref<Rule>> Parser::ConsumeDeclarationRuleListInNewNestingContext(TokenRange tokens,
                                                                        RuleType ruleType) noexcept
  {
    List<Ref<Rule>> rules;
    RunInNewNestingContext(
      [&]
      {
        ConsumeDeclarationRuleList(tokens, ruleType);
        rules.insert(rules.end(), TopContext().ParsedRules.begin(), TopContext().ParsedRules.end());
      });
    return rules;
  }

  void Parser::ConsumeDeclarationList(TokenRange tokens, RuleType ruleType) noexcept
  {
    // https://drafts.csswg.org/css-syntax-3/#block-contents
    // <declaration-list>: only declarations are allowed; at-rules and qualified rules are automatically
    // invalid.
    ConsumeBlockContent(tokens, ruleType, BlockAllowedRule::Declarations);
  }

  void Parser::ConsumeDeclarationRuleList(TokenRange tokens, RuleType ruleType) noexcept
  {
    // <declaration-rule-list>: declarations and at-rules are allowed; qualified rules are automatically
    // invalid.
    ConsumeBlockContent(tokens, ruleType, {BlockAllowedRule::Declarations, BlockAllowedRule::AtRules});
  }

  void Parser::ConsumeStyleBlock(TokenRange tokens, RuleType ruleType) noexcept
  {
    // <block-contents>
    ConsumeBlockContent(
      tokens, ruleType,
      {BlockAllowedRule::Declarations, BlockAllowedRule::QualifiedRules, BlockAllowedRule::AtRules});
  }

  IsImportant Parser::ConsumeTrailingImportantAndWhitespace(TokenRange &tokens) noexcept
  {
    tokens.DiscardTrailingWhitespace();
    if (tokens.Size() < 2)
    {
      return IsImportant(false);
    }

    auto removeImportantRange = tokens;
    if (auto &last = removeImportantRange.ConsumeLast();
        last.Type() != TokenType::Ident
        || !text::IsASCIICaselessEqual(last.IdentCodePoints(), utf8_stringview(u8"important")))
    {
      return IsImportant(false);
    }

    removeImportantRange.DiscardTrailingWhitespace();
    if (auto &last = removeImportantRange.ConsumeLast();
        last.Type() != TokenType::Delim || last.Delimiter() != U'!')
    {
      return IsImportant(false);
    }

    removeImportantRange.DiscardTrailingWhitespace();
    tokens = removeImportantRange;
    return IsImportant(true);
  }

  namespace
  {
    // Check if a CSS rule type does not allow declarations with !important.
    bool RuleDoesNotAllowImportant(RuleType type) noexcept
    {
      return type == RuleType::CounterStyle || type == RuleType::FontFace
             || type == RuleType::FontPaletteValues || type == RuleType::Keyframe
             || type == RuleType::PositionTry || type == RuleType::ViewTransition
             || type == RuleType::Function;
    }
  }

  // https://drafts.csswg.org/css-syntax/#consume-declaration
  bool Parser::ConsumeDeclaration(TokenRange tokens, RuleType ruleType) noexcept
  {
    krys_debug_assert(tokens.Peek().Type() == TokenType::Ident);
    auto &token = tokens.ConsumeIncludingWhitespace();
    auto property = token.PropertyId();
    if (tokens.Consume().Type() != TokenType::Colon)
    {
      return false; // Parse error
    }

    tokens.DiscardWhitespace();

    auto important = ConsumeTrailingImportantAndWhitespace(tokens);
    if (important && RuleDoesNotAllowImportant(ruleType))
    {
      return false;
    }

    const size_t oldPropertiesCount = TopContext().ParsedProperties.size();
    auto didParseNewProperties = [&]
    {
      return TopContext().ParsedProperties.size() != oldPropertiesCount;
    };

    if (!IsExposed(property, &_context.PropertySettings))
    {
      property = PropertyId::Invalid;
    }

    // @position-try doesn't allow custom properties.
    // FIXME: maybe make this logic more elegant?
    if (property == PropertyId::Invalid && VariableParser::IsValidVariableName(token)
        && ruleType != RuleType::PositionTry)
    {
      auto variableName = CSSOMStringAtom(token.IdentCodePoints());
      ConsumeCustomPropertyValue(tokens, variableName, important);
    }

    if (property != PropertyId::Invalid)
    {
      ConsumeDeclarationValue(tokens, property, important, ruleType);
    }

    return didParseNewProperties();
  }

  void Parser::ConsumeCustomPropertyValue(TokenRange tokens, const CSSOMStringAtom &variableName,
                                          IsImportant important) noexcept
  {
    if (tokens.IsAtEnd())
    {
      TopContext().ParsedProperties.push_back(
        Property(PropertyId::Custom, CustomPropertyValue::CreateEmpty(variableName), important));
    }
    else if (auto value = VariableParser::ParseDeclarationValue(variableName, tokens, _context))
    {
      TopContext().ParsedProperties.push_back(Property(PropertyId::Custom, ShareRef(*value), important));
    }
  }

  void Parser::ConsumeDeclarationValue(TokenRange tokens, PropertyId property, IsImportant important,
                                       RuleType ruleType) noexcept
  {
    std::ignore = PropertyParser::ParseValue(property, important, tokens, _context,
                                             TopContext().ParsedProperties, ruleType);
  }
}
