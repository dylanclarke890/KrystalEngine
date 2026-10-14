#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorInlines.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorPseudoTypeMap.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <charconv>

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD CSSOMStringAtom SerialiseANPlusB(const Pair<int32, int32> &ab) noexcept;
    KRYS_NODISCARD bool ConsumeANPlusB(TokenRange &tokens, Pair<int32, int32> &result) noexcept;

    void AppendImplicitSelectorPseudoClassScopeIfNeeded(MutableSelector &selector) noexcept
    {
      if ((!selector.HasExplicitNestingParent() && !selector.HasExplicitPseudoClassScope())
          || selector.StartsWithExplicitCombinator())
      {
        auto scopeSelector = CreateUnique<MutableSelector>();
        scopeSelector->SetMatch(SelectorMatch::PseudoClass);
        scopeSelector->SetPseudoClass(SelectorPseudoClass::Scope);
        scopeSelector->SetImplicit();
        selector.PrependComplexSelectorComponentAsRelative(krys::move(scopeSelector));
      }
    }

    void AppendImplicitSelectorNestingParentIfNeeded(MutableSelector &selector) noexcept
    {
      if (!selector.HasExplicitNestingParent() || selector.StartsWithExplicitCombinator())
      {
        auto nestingParentSelector = CreateUnique<MutableSelector>();
        nestingParentSelector->SetMatch(SelectorMatch::NestingParent);
        // https://drafts.csswg.org/css-nesting/#nesting
        // Spec: nested rules with relative selectors include the specificity of their implied nesting
        // selector.
        selector.PrependComplexSelectorComponentAsRelative(krys::move(nestingParentSelector));
      }
    }

    void AppendImplicitSelectorIfNeeded(MutableSelector &selector, NestedParserContextType last) noexcept
    {
      if (last == NestedParserContextType::Style)
      {
        // For a rule inside a style rule, we add the implicit & if it's not there already or if it starts
        // with a combinator > ~ +
        AppendImplicitSelectorNestingParentIfNeeded(selector);
      }
      else if (last == NestedParserContextType::Scope)
      {
        // For a rule inside a scope rule, we add the implicit ":scope" if there is no explicit & or :scope
        // already
        AppendImplicitSelectorPseudoClassScopeIfNeeded(selector);
      }
    }

    KRYS_NODISCARD bool IsDelimiter(const Token &token, char32 delimiter) noexcept
    {
      return token.Type() == TokenType::Delim && token.Delimiter() == delimiter;
    }

    KRYS_NODISCARD int32 ClampToInt32(double value) noexcept
    {
      return static_cast<int32>(std::clamp(value, static_cast<double>(std::numeric_limits<int32>::min()),
                                           static_cast<double>(std::numeric_limits<int32>::max())));
    }
  }

  MutableSelectorList ParseMutableSelectorList(TokenRange &tokens, const SelectorParserContext &context,
                                               StyleSheetContents *styleSheet, NestedContext nestedContext,
                                               IsForgivingSelectorList isForgiving,
                                               DisallowPseudoElements disallowPseudoElements) noexcept
  {
    SelectorParser parser(context, styleSheet, nestedContext, disallowPseudoElements);
    tokens.DiscardWhitespace();
    auto consume = [&]
    {
      if (nestedContext && !isForgiving)
      {
        return parser.ConsumeNestedSelectorList(tokens);
      }
      if (nestedContext && isForgiving)
      {
        return parser.ConsumeNestedComplexForgivingSelectorList(tokens);
      }
      if (isForgiving)
      {
        return parser.ConsumeComplexForgivingSelectorList(tokens);
      }
      return parser.ConsumeComplexSelectorList(tokens);
    };
    auto result = consume();
    if (result.empty() || !tokens.IsAtEnd())
    {
      return {};
    }

    // In nested context, add the implicit :scope or &
    if (nestedContext)
    {
      for (auto &selector : result)
      {
        AppendImplicitSelectorIfNeeded(*selector, *nestedContext);
      }
    }

    return result;
  }

  Maybe<SelectorList> ParseSelectorList(TokenRange tokens, const SelectorParserContext &context,
                                        StyleSheetContents *styleSheet, NestedContext nestedContext) noexcept
  {
    auto result = ParseMutableSelectorList(tokens, context, styleSheet, nestedContext,
                                           IsForgivingSelectorList(false), DisallowPseudoElements(false));

    if (result.empty() || !tokens.IsAtEnd())
    {
      return {};
    }

    return SelectorList {krys::move(result)};
  }

  SelectorParser::SelectorParser(const SelectorParserContext &context, StyleSheetContents *styleSheet,
                                 NestedContext nestedContext,
                                 DisallowPseudoElements disallowPseudoElements) noexcept
      : _context(context), _styleSheet(ShareRefPtr(styleSheet)), _nestedContext(nestedContext),
        _disallowPseudoElements(disallowPseudoElements)
  {
  }

  template <typename TConsumeSelector>
  MutableSelectorList SelectorParser::ConsumeSelectorList(TokenRange &tokens,
                                                          TConsumeSelector &&consumeSelector) noexcept
  {
    MutableSelectorList selectorList;
    auto selector = consumeSelector(tokens);
    if (!selector)
    {
      return {};
    }

    selectorList.push_back(krys::move(selector));
    while (!tokens.IsAtEnd() && tokens.Peek().Type() == TokenType::Comma)
    {
      std::ignore = tokens.ConsumeIncludingWhitespace();
      selector = consumeSelector(tokens);
      if (!selector)
      {
        return {};
      }
      selectorList.push_back(krys::move(selector));
    }

    if (_failedParsing)
    {
      return {};
    }

    return selectorList;
  }

  MutableSelectorList SelectorParser::ConsumeComplexSelectorList(TokenRange &tokens) noexcept
  {
    return ConsumeSelectorList(tokens, [&](TokenRange &range) { return ConsumeComplexSelector(range); });
  }

  MutableSelectorList SelectorParser::ConsumeRelativeSelectorList(TokenRange &tokens) noexcept
  {
    return ConsumeSelectorList(tokens,
                               [&](TokenRange &range) { return ConsumeRelativeScopeSelector(range); });
  }

  MutableSelectorList SelectorParser::ConsumeNestedSelectorList(TokenRange &tokens) noexcept
  {
    return ConsumeSelectorList(tokens,
                               [&](TokenRange &range) { return ConsumeNestedComplexSelector(range); });
  }

  template <typename TConsumeSelector>
  MutableSelectorList
    SelectorParser::ConsumeForgivingSelectorList(TokenRange &tokens,
                                                 TConsumeSelector &&consumeSelector) noexcept
  {
    if (_failedParsing)
    {
      return {};
    }

    MutableSelectorList selectorList;

    auto consumeForgiving = [&]
    {
      auto initialRange = tokens;
      auto unknownSelector = [&]
      {
        auto unknownSelector = CreateUnique<MutableSelector>();
        auto unknownRange = initialRange.RangeUntil(tokens);
        unknownSelector->SetMatch(SelectorMatch::ForgivingUnknown);
        // We store the complete range content for serialisation.
        unknownSelector->SetValue(CSSOMStringAtom(unknownRange.Serialise(TokenSerialisationMode::Normal)));
        // If the range contains a nesting selector, we mark this unknown selector as "nest containing" (it
        // will be used during rule set building)
        for (const auto &token : unknownRange.Tokens())
        {
          if (IsDelimiter(token, U'&'))
          {
            unknownSelector->SetMatch(SelectorMatch::ForgivingUnknownNestContaining);
            break;
          }
        }
        return unknownSelector;
      };

      auto selector = consumeSelector(tokens);

      if (_failedParsing && !_disableForgivingParsing)
      {
        selector = nullptr;
        _failedParsing = false;
      }

      // Range is not over and next token is not a comma (means there is more to this selector) so this
      // selector is unknown. Consume until next comma and add the full range as an unknown selector to the
      // selector list.
      if ((!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Comma) || !selector)
      {
        while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Comma)
        {
          std::ignore = tokens.Consume();
        }
        if (!_disableForgivingParsing)
        {
          selectorList.push_back(unknownSelector());
        }
        return;
      }

      selectorList.push_back(krys::move(selector));
    };

    consumeForgiving();

    while (!tokens.IsAtEnd() && tokens.Peek().Type() == TokenType::Comma)
    {
      std::ignore = tokens.ConsumeIncludingWhitespace();
      consumeForgiving();
    }

    if (selectorList.empty())
    {
      if (_disableForgivingParsing)
      {
        _failedParsing = true;
      }
      return {};
    }

    return selectorList;
  }

  MutableSelectorList SelectorParser::ConsumeComplexForgivingSelectorList(TokenRange &tokens) noexcept
  {
    return ConsumeForgivingSelectorList(tokens,
                                        [&](TokenRange &range) { return ConsumeComplexSelector(range); });
  }

  MutableSelectorList SelectorParser::ConsumeNestedComplexForgivingSelectorList(TokenRange &tokens) noexcept
  {
    return ConsumeForgivingSelectorList(tokens, [&](TokenRange &range)
                                        { return ConsumeNestedComplexSelector(range); });
  }

  Maybe<SelectorList> SelectorParser::ParseSelectorList(CSSOMStringView string,
                                                        const SelectorParserContext &context,
                                                        StyleSheetContents *styleSheet,
                                                        NestedContext nestedContext) noexcept
  {
    InputStream inputStream {CSSOMString(string)};
    Tokenizer tokenizer(inputStream);
    std::ignore = tokenizer.PumpTokenizer();
    return css::ParseSelectorList(tokenizer.Tokens(), context, styleSheet, nestedContext);
  }

  bool SelectorParser::SupportsComplexSelector(TokenRange tokens,
                                               const SelectorParserContext &context) noexcept
  {
    tokens.DiscardWhitespace();
    SelectorParser parser(context, nullptr);

    // @supports requires that all arguments parse.
    parser._disableForgivingParsing = true;

    auto mutableSelector = parser.ConsumeComplexSelector(tokens);

    return !parser._failedParsing && tokens.IsAtEnd() && mutableSelector;
  }

  MutableSelectorList SelectorParser::ConsumeCompoundSelectorList(TokenRange &tokens) noexcept
  {
    MutableSelectorList selectorList;
    auto selector = ConsumeCompoundSelector(tokens);
    tokens.DiscardWhitespace();
    if (!selector)
    {
      return {};
    }
    selectorList.push_back(krys::move(selector));
    while (!tokens.IsAtEnd() && tokens.Peek().Type() == TokenType::Comma)
    {
      std::ignore = tokens.ConsumeIncludingWhitespace();
      selector = ConsumeCompoundSelector(tokens);
      tokens.DiscardWhitespace();
      if (!selector)
      {
        return {};
      }
      selectorList.push_back(krys::move(selector));
    }
    return selectorList;
  }

  namespace
  {
    KRYS_NODISCARD PossiblyQuotedIdentifier ConsumePossiblyQuotedIdentifier(TokenRange &tokens) noexcept
    {
      const auto &token = tokens.ConsumeIncludingWhitespace();
      if (token.Type() != TokenType::Ident && token.Type() != TokenType::String)
      {
        return {};
      }
      auto string = token.IdentCodePoints();
      if (string.starts_with(u8"--"))
      {
        return {};
      }
      return {CSSOMStringAtom(string), token.Type() == TokenType::String};
    }

    KRYS_NODISCARD FixedVector<PossiblyQuotedIdentifier> ConsumeLangArgumentList(TokenRange &tokens) noexcept
    {
      List<PossiblyQuotedIdentifier> list;
      auto item = ConsumePossiblyQuotedIdentifier(tokens);
      if (item.IsNull())
      {
        return {};
      }
      list.push_back(krys::move(item));
      while (!tokens.IsAtEnd() && tokens.Peek().Type() == TokenType::Comma)
      {
        std::ignore = tokens.ConsumeIncludingWhitespace();
        item = ConsumePossiblyQuotedIdentifier(tokens);
        if (item.IsNull())
        {
          return {};
        }
        list.push_back(krys::move(item));
      }
      return FixedVector<PossiblyQuotedIdentifier>(list.begin(), list.end());
    }

    KRYS_NODISCARD Maybe<FixedVector<CSSOMStringAtom>>
      ConsumeCommaSeparatedCustomIdentList(TokenRange &tokens) noexcept
    {
      List<CSSOMStringAtom> customIdents;

      do
      {
        auto ident = PropertyParserHelpers::ConsumeCustomIdentRaw(tokens);
        if (!ident || ident->empty())
        {
          return {};
        }

        customIdents.push_back(CSSOMStringAtom(*ident));
      } while (PropertyParserHelpers::ConsumeComma(tokens));

      if (!tokens.IsAtEnd())
      {
        return {};
      }

      // The parsing code guarantees there has to be at least one custom ident.
      krys_debug_assert(!customIdents.empty());

      return FixedVector<CSSOMStringAtom>(customIdents.begin(), customIdents.end());
    }

    /// @brief Whether a compound has a pseudo-element, so no compound may follow it.
    KRYS_NODISCARD bool HasPseudoElementForRightmostCompound(const MutableSelector &simpleSelector,
                                                             ParserMode parserMode) noexcept
    {
      if (simpleSelector.Match() != SelectorMatch::PseudoElement)
      {
        return false;
      }

      // FIXME: https://bugs.webkit.org/show_bug.cgi?id=161747
      // The UASheetMode check is a work-around to allow this selector in mediaControls(New).css:
      // input[type="range" i]::-webkit-media-slider-container > div {
      if (IsUASheetBehavior(parserMode)
          && simpleSelector.PseudoElement() == SelectorPseudoElement::UserAgentPart)
      {
        return false;
      }

      return true;
    }
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeNestedComplexSelector(TokenRange &tokens) noexcept
  {
    auto selector = ConsumeComplexSelector(tokens);
    if (selector)
    {
      return selector;
    }

    selector = ConsumeRelativeNestedSelector(tokens);
    if (selector)
    {
      return selector;
    }

    return nullptr;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeComplexSelector(TokenRange &tokens) noexcept
  {
    auto selector = ConsumeCompoundSelector(tokens);
    if (!selector)
    {
      return nullptr;
    }

    bool previousCompoundHasPseudoElement = false;

    for (auto *simple = selector.get(); simple && !previousCompoundHasPseudoElement;
         simple = simple->PrecedingComplexSelectorComponent())
    {
      previousCompoundHasPseudoElement = HasPseudoElementForRightmostCompound(*simple, _context.Mode);
    }

    while (true)
    {
      auto combinator = ConsumeCombinator(tokens);
      if (combinator == SelectorRelation::Compounding)
      {
        break;
      }

      auto nextSelector = ConsumeCompoundSelector(tokens);
      if (!nextSelector)
      {
        return combinator == SelectorRelation::Descendant ? krys::move(selector) : nullptr;
      }
      if (previousCompoundHasPseudoElement)
      {
        return nullptr;
      }
      auto *end = nextSelector.get();
      auto compoundHasPseudoElement = HasPseudoElementForRightmostCompound(*end, _context.Mode);
      while (end->PrecedingComplexSelectorComponent())
      {
        end = end->PrecedingComplexSelectorComponent();
        compoundHasPseudoElement |= HasPseudoElementForRightmostCompound(*end, _context.Mode);
      }
      end->SetRelation(combinator);
      previousCompoundHasPseudoElement = compoundHasPseudoElement;
      end->SetPrecedingComplexSelectorComponent(krys::move(selector));

      selector = krys::move(nextSelector);
    }

    return selector;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeRelativeScopeSelector(TokenRange &tokens) noexcept
  {
    auto scopeCombinator = ConsumeCombinator(tokens);

    if (scopeCombinator == SelectorRelation::Compounding)
    {
      scopeCombinator = SelectorRelation::Descendant;
    }

    auto selector = ConsumeComplexSelector(tokens);
    if (!selector)
    {
      return nullptr;
    }

    auto *end = selector.get();
    while (end->PrecedingComplexSelectorComponent())
    {
      end = end->PrecedingComplexSelectorComponent();
    }

    auto scopeSelector = CreateUnique<MutableSelector>();
    scopeSelector->SetMatch(SelectorMatch::HasScope);

    end->SetRelation(scopeCombinator);
    end->SetPrecedingComplexSelectorComponent(krys::move(scopeSelector));

    return selector;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeRelativeNestedSelector(TokenRange &tokens) noexcept
  {
    auto scopeCombinator = ConsumeCombinator(tokens);

    // Nesting should only work with ~ > + combinators in this function.
    // The descendant combinator is handled in another code path.
    if (scopeCombinator != SelectorRelation::NextSibling
        && scopeCombinator != SelectorRelation::SubsequentSibling
        && scopeCombinator != SelectorRelation::Child)
    {
      return nullptr;
    }

    auto selector = ConsumeComplexSelector(tokens);
    if (!selector)
    {
      return nullptr;
    }

    auto *last = selector->LeftmostSimpleSelector();
    last->SetRelation(scopeCombinator);

    return selector;
  }

  namespace
  {
    KRYS_NODISCARD bool IsUserActionPseudoClass(SelectorPseudoClass pseudo) noexcept
    {
      switch (pseudo)
      {
        case SelectorPseudoClass::Hover:
        case SelectorPseudoClass::Focus:
        case SelectorPseudoClass::Active:
        case SelectorPseudoClass::FocusVisible:
        case SelectorPseudoClass::FocusWithin:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

    KRYS_NODISCARD bool
      IsPseudoClassValidAfterPseudoElement(SelectorPseudoClass pseudoClass,
                                           SelectorPseudoElement compoundPseudoElement) noexcept
    {
      // FIXME: https://drafts.csswg.org/selectors-4/#pseudo-element-states states all pseudo-elements can be
      // followed by IsUserActionPseudoClass(). Validity of these is determined by their content.
      if (IsLogicalCombinationPseudoClass(pseudoClass))
      {
        return true;
      }

      switch (compoundPseudoElement)
      {
        case SelectorPseudoElement::Part:
        {
          return !IsTreeStructuralPseudoClass(pseudoClass);
        }
        case SelectorPseudoElement::ViewTransitionGroup:
        case SelectorPseudoElement::ViewTransitionImagePair:
        case SelectorPseudoElement::ViewTransitionNew:
        case SelectorPseudoElement::ViewTransitionOld:
        {
          return pseudoClass == SelectorPseudoClass::OnlyChild;
        }
        case SelectorPseudoElement::UserAgentPart:
        {
          return IsUserActionPseudoClass(pseudoClass);
        }
        default:
        {
          return false;
        }
      }
    }

    KRYS_NODISCARD bool IsTreeAbidingPseudoElement(SelectorPseudoElement pseudoElement) noexcept
    {
      switch (pseudoElement)
      {
        // FIXME: This list should also include ::placeholder and ::file-selector-button
        case SelectorPseudoElement::Before:
        case SelectorPseudoElement::After:
        case SelectorPseudoElement::Marker:
        case SelectorPseudoElement::Backdrop:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

    KRYS_NODISCARD bool
      IsSimpleSelectorValidAfterPseudoElement(const MutableSelector &simpleSelector,
                                              const MutableSelector &compoundPseudoElement) noexcept
    {
      if (compoundPseudoElement.PseudoElement() == SelectorPseudoElement::UserAgentPart
          && compoundPseudoElement.Value() == UserAgentParts::DetailsContent())
      {
        if (simpleSelector.Match() == SelectorMatch::PseudoElement)
        {
          return true;
        }
      }
      if (compoundPseudoElement.PseudoElement() == SelectorPseudoElement::Part)
      {
        if (simpleSelector.Match() == SelectorMatch::PseudoElement
            && simpleSelector.PseudoElement() != SelectorPseudoElement::Part)
        {
          return true;
        }
      }
      if (compoundPseudoElement.PseudoElement() == SelectorPseudoElement::Slotted)
      {
        if (simpleSelector.Match() == SelectorMatch::PseudoElement
            && IsTreeAbidingPseudoElement(simpleSelector.PseudoElement()))
        {
          return true;
        }
      }
      if (simpleSelector.Match() != SelectorMatch::PseudoClass)
      {
        return false;
      }

      return IsPseudoClassValidAfterPseudoElement(simpleSelector.PseudoClass(),
                                                  compoundPseudoElement.PseudoElement());
    }

    KRYS_NODISCARD bool AtEndIgnoringWhitespace(TokenRange tokens) noexcept
    {
      tokens.DiscardWhitespace();
      return tokens.IsAtEnd();
    }
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeCompoundSelector(TokenRange &tokens) noexcept
  {
    krys_debug_assert(!_precedingPseudoElement || _disallowPseudoElements);

    UniquePtr<MutableSelector> compoundSelector;

    CSSOMStringAtom namespacePrefix = CSSOMStringAtom::Null();
    CSSOMStringAtom elementName = CSSOMStringAtom::Null();
    const bool hasName = ConsumeName(tokens, elementName, namespacePrefix);
    if (!hasName)
    {
      compoundSelector = ConsumeSimpleSelector(tokens);
      if (!compoundSelector)
      {
        return nullptr;
      }
      if (compoundSelector->Match() == SelectorMatch::PseudoElement)
      {
        _precedingPseudoElement = compoundSelector.get();
      }
    }

    while (auto simpleSelector = ConsumeSimpleSelector(tokens))
    {
      if (simpleSelector->Match() == SelectorMatch::PseudoElement)
      {
        _precedingPseudoElement = simpleSelector.get();
      }

      if (compoundSelector)
      {
        compoundSelector->PrependComplexSelectorComponent(SelectorRelation::Compounding,
                                                          krys::move(simpleSelector));
      }
      else
      {
        compoundSelector = krys::move(simpleSelector);
      }
    }

    if (!_disallowPseudoElements)
    {
      _precedingPseudoElement = nullptr;
    }

    // While inside a nested selector like :is(), the default namespace shall be ignored when [1]:
    // * The compound selector represents the subject [2], and
    // * The compound selector does not contain a type/universal selector.
    //
    // [1] https://drafts.csswg.org/selectors/#matches
    // [2] https://drafts.csswg.org/selectors/#selector-subject
    SetForScope ignoreDefaultNamespace(_ignoreDefaultNamespace, _resistDefaultNamespace && !hasName
                                                                  && AtEndIgnoringWhitespace(tokens));
    if (!compoundSelector)
    {
      CSSOMStringAtom namespaceURI = DetermineNamespace(namespacePrefix);
      if (namespaceURI == CSSOMStringAtom::Null())
      {
        _failedParsing = true;
        return nullptr;
      }
      if (namespaceURI == DefaultNamespace())
      {
        namespacePrefix = CSSOMStringAtom::Null();
      }

      return CreateUnique<MutableSelector>(dom::QualifiedName(namespaceURI, namespacePrefix, elementName));
    }
    PrependTypeSelectorIfNeeded(namespacePrefix, elementName, *compoundSelector);
    return SplitCompoundAtImplicitShadowCrossingCombinator(krys::move(compoundSelector), _context);
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeSimpleSelector(TokenRange &tokens) noexcept
  {
    const auto &token = tokens.Peek();
    UniquePtr<MutableSelector> selector;
    if (token.Type() == TokenType::Hash)
    {
      selector = ConsumeId(tokens);
    }
    else if (IsDelimiter(token, U'.'))
    {
      selector = ConsumeClass(tokens);
    }
    else if (IsDelimiter(token, U'&'))
    {
      selector = ConsumeNesting(tokens);
    }
    else if (token.Type() == TokenType::OpenSquare)
    {
      selector = ConsumeAttribute(tokens);
    }
    else if (token.Type() == TokenType::Colon)
    {
      selector = ConsumePseudo(tokens);
    }
    else
    {
      return nullptr;
    }

    if (!selector)
    {
      _failedParsing = true;
      return nullptr;
    }

    if (_precedingPseudoElement)
    {
      // FIXME: https://bugs.webkit.org/show_bug.cgi?id=161747
      // The UASheetMode check is a work-around to allow this selector in mediaControls(New).css:
      // video::-webkit-media-text-track-region-container.scrolling
      if (!IsUASheetBehavior(_context.Mode)
          && !IsSimpleSelectorValidAfterPseudoElement(*selector, *_precedingPseudoElement))
      {
        _failedParsing = true;
      }
    }

    return selector;
  }

  bool SelectorParser::ConsumeName(TokenRange &tokens, CSSOMStringAtom &name,
                                   CSSOMStringAtom &namespacePrefix) noexcept
  {
    name = CSSOMStringAtom::Null();
    namespacePrefix = CSSOMStringAtom::Null();

    const auto &firstToken = tokens.Peek();
    if (firstToken.Type() == TokenType::Ident)
    {
      name = CSSOMStringAtom(firstToken.IdentCodePoints());
      std::ignore = tokens.Consume();
    }
    else if (IsDelimiter(firstToken, U'*'))
    {
      name = dom::StarAtom();
      std::ignore = tokens.Consume();
    }
    else if (IsDelimiter(firstToken, U'|'))
    {
      // This is an empty namespace, which'll get assigned this value below
      name = CSSOMStringAtom::Empty();
    }
    else
    {
      return false;
    }

    // `|=` is the dash-match attribute operator, not a namespace separator.
    if (!IsDelimiter(tokens.Peek(), U'|') || IsDelimiter(tokens.Peek(1uz), U'='))
    {
      if (name == CSSOMStringAtom::Empty())
      {
        name = CSSOMStringAtom::Null();
        return false;
      }
      return true;
    }

    namespacePrefix = name;
    if (tokens.Peek(1uz).Type() == TokenType::Ident)
    {
      std::ignore = tokens.Consume();
      name = CSSOMStringAtom(tokens.Consume().IdentCodePoints());
    }
    else if (IsDelimiter(tokens.Peek(1uz), U'*'))
    {
      std::ignore = tokens.Consume();
      std::ignore = tokens.Consume();
      name = dom::StarAtom();
    }
    else
    {
      name = CSSOMStringAtom::Null();
      namespacePrefix = CSSOMStringAtom::Null();
      return false;
    }

    return true;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeId(TokenRange &tokens) noexcept
  {
    krys_debug_assert(tokens.Peek().Type() == TokenType::Hash);
    if (tokens.Peek().HashType() != HashTokenType::Id)
    {
      return nullptr;
    }

    auto selector = CreateUnique<MutableSelector>();
    selector->SetMatch(SelectorMatch::Id);

    const auto &token = tokens.Consume();
    selector->SetValue(CSSOMStringAtom(token.IdentCodePoints()),
                       MatchLowercase(_context.Mode == ParserMode::HTMLQuirks));
    return selector;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeClass(TokenRange &tokens) noexcept
  {
    krys_debug_assert(IsDelimiter(tokens.Peek(), U'.'));
    std::ignore = tokens.Consume();
    if (tokens.Peek().Type() != TokenType::Ident)
    {
      return nullptr;
    }

    auto selector = CreateUnique<MutableSelector>();
    selector->SetMatch(SelectorMatch::Class);

    const auto &token = tokens.Consume();
    selector->SetValue(CSSOMStringAtom(token.IdentCodePoints()),
                       MatchLowercase(_context.Mode == ParserMode::HTMLQuirks));

    return selector;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeNesting(TokenRange &tokens) noexcept
  {
    krys_debug_assert(IsDelimiter(tokens.Peek(), U'&'));
    std::ignore = tokens.Consume();

    auto selector = CreateUnique<MutableSelector>();
    selector->SetMatch(SelectorMatch::NestingParent);

    return selector;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeAttribute(TokenRange &tokens) noexcept
  {
    krys_debug_assert(tokens.Peek().Type() == TokenType::OpenSquare);
    TokenRange block = tokens.ConsumeBlock();
    block.DiscardWhitespace();

    CSSOMStringAtom namespacePrefix = CSSOMStringAtom::Null();
    CSSOMStringAtom attributeName = CSSOMStringAtom::Null();
    if (!ConsumeName(block, attributeName, namespacePrefix))
    {
      return nullptr;
    }
    block.DiscardWhitespace();

    CSSOMStringAtom namespaceURI = DetermineNamespace(namespacePrefix);
    if (namespaceURI == CSSOMStringAtom::Null())
    {
      return nullptr;
    }

    dom::QualifiedName qualifiedName =
      namespacePrefix == CSSOMStringAtom::Null()
        ? dom::QualifiedName(CSSOMStringAtom::Null(), CSSOMStringAtom::Null(), attributeName)
        : dom::QualifiedName(namespaceURI, namespacePrefix, attributeName);

    auto selector = CreateUnique<MutableSelector>();

    if (block.IsAtEnd())
    {
      selector->SetAttribute(qualifiedName, IsCaseSensitive(true));
      selector->SetMatch(SelectorMatch::AttributeExists);
      return selector;
    }

    selector->SetMatch(ConsumeAttributeMatch(block));

    const auto &attributeValue = block.ConsumeIncludingWhitespace();
    if (attributeValue.Type() != TokenType::Ident && attributeValue.Type() != TokenType::String)
    {
      return nullptr;
    }
    selector->SetValue(CSSOMStringAtom(attributeValue.IdentCodePoints()));

    selector->SetAttribute(qualifiedName, ConsumeAttributeFlags(block));

    if (!block.IsAtEnd())
    {
      return nullptr;
    }
    return selector;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumePseudo(TokenRange &tokens) noexcept
  {
    krys_debug_assert(tokens.Peek().Type() == TokenType::Colon);
    std::ignore = tokens.Consume();

    int32 colons = 1;
    if (tokens.Peek().Type() == TokenType::Colon)
    {
      std::ignore = tokens.Consume();
      colons++;
    }

    const auto &token = tokens.Peek();
    if (token.Type() != TokenType::Ident && token.Type() != TokenType::Function)
    {
      return nullptr;
    }

    UniquePtr<MutableSelector> selector;

    if (colons == 1)
    {
      selector = MutableSelector::ParsePseudoClassSelector(token.IdentCodePoints(), _context);
    }
    else
    {
      selector = MutableSelector::ParsePseudoElementSelector(token.IdentCodePoints(), _context);

      // Treat the ident version of cue as SelectorPseudoElement::UserAgentPart.
      if (token.Type() == TokenType::Ident && selector && selector->Match() == SelectorMatch::PseudoElement
          && selector->PseudoElement() == SelectorPseudoElement::Cue)
      {
        selector->SetPseudoElement(SelectorPseudoElement::UserAgentPart);
      }
    }

    if (!selector)
    {
      return nullptr;
    }

    // Pseudo-elements are not allowed inside pseudo-classes or pseudo-elements.
    if (selector->Match() == SelectorMatch::PseudoElement && _disallowPseudoElements)
    {
      return nullptr;
    }
    SetForScope disallowPseudoElementsScope(_disallowPseudoElements, true);

    if (token.Type() == TokenType::Ident)
    {
      std::ignore = tokens.Consume();
      if ((selector->Match() == SelectorMatch::PseudoElement
           && Selector::PseudoElementRequiresArgument(selector->PseudoElement()))
          || (selector->Match() == SelectorMatch::PseudoClass
              && Selector::PseudoClassRequiresArgument(selector->PseudoClass())))
      {
        return nullptr;
      }
      return selector;
    }

    krys_debug_assert(token.Type() == TokenType::Function);
    TokenRange block = tokens.ConsumeBlock();
    block.DiscardWhitespace();

    if (selector->Match() == SelectorMatch::PseudoClass)
    {
      switch (selector->PseudoClass())
      {
        case SelectorPseudoClass::Not:
        {
          SetForScope resistDefaultNamespace(_resistDefaultNamespace, true);
          auto selectorList = ConsumeComplexSelectorList(block);
          if (selectorList.size() < 1 || !block.IsAtEnd())
          {
            return nullptr;
          }
          selector->SetSubSelectors(CreateUnique<SelectorList>(krys::move(selectorList)));
          return selector;
        }
        case SelectorPseudoClass::NthChild:
        case SelectorPseudoClass::NthLastChild:
        case SelectorPseudoClass::NthOfType:
        case SelectorPseudoClass::NthLastOfType:
        {
          Pair<int32, int32> ab;
          if (!ConsumeANPlusB(block, ab))
          {
            return nullptr;
          }
          block.DiscardWhitespace();
          // FIXME: We should be able to do this lazily. See: https://bugs.webkit.org/show_bug.cgi?id=217149
          selector->SetArgument(SerialiseANPlusB(ab));
          if (!block.IsAtEnd())
          {
            auto type = selector->PseudoClass();
            if (type == SelectorPseudoClass::NthOfType || type == SelectorPseudoClass::NthLastOfType)
            {
              return nullptr;
            }
            if (block.Peek().Type() != TokenType::Ident)
            {
              return nullptr;
            }
            const auto &ident = block.Consume();
            if (!text::IsASCIICaselessEqual(ident.IdentCodePoints(), CSSOMStringView {u8"of"}))
            {
              return nullptr;
            }
            block.DiscardWhitespace();
            auto selectorList = ConsumeComplexSelectorList(block);
            if (selectorList.empty() || !block.IsAtEnd())
            {
              return nullptr;
            }
            selector->SetSubSelectors(CreateUnique<SelectorList>(krys::move(selectorList)));
          }
          selector->SetNth(ab.first, ab.second);
          return selector;
        }
        case SelectorPseudoClass::Lang:
        {
          auto list = ConsumeLangArgumentList(block);
          if (list.isEmpty() || !block.IsAtEnd())
          {
            return nullptr;
          }
          selector->SetLangList(krys::move(list));
          return selector;
        }
        case SelectorPseudoClass::Is:
        case SelectorPseudoClass::Where:
        {
          SetForScope resistDefaultNamespace(_resistDefaultNamespace, true);
          auto consumedBlock = ConsumeComplexForgivingSelectorList(block);
          if (!block.IsAtEnd())
          {
            return nullptr;
          }
          if (consumedBlock.empty())
          {
            selector->SetSubSelectors(CreateUnique<SelectorList>());
          }
          else
          {
            selector->SetSubSelectors(CreateUnique<SelectorList>(krys::move(consumedBlock)));
          }
          return selector;
        }
        case SelectorPseudoClass::Host:
        {
          auto innerSelector = ConsumeCompoundSelector(block);
          block.DiscardWhitespace();
          if (!innerSelector || !block.IsAtEnd())
          {
            return nullptr;
          }
          MutableSelectorList list;
          list.push_back(krys::move(innerSelector));
          selector->AdoptSelectorVector(krys::move(list));
          return selector;
        }
        case SelectorPseudoClass::Has:
        {
          if (_disallowHasPseudoClass)
          {
            return nullptr;
          }
          SetForScope resistDefaultNamespace(_resistDefaultNamespace, true);
          SetForScope disallowNestedHas(_disallowHasPseudoClass, true);
          auto selectorList = ConsumeRelativeSelectorList(block);
          if (selectorList.empty() || !block.IsAtEnd())
          {
            return nullptr;
          }
          selector->SetSubSelectors(CreateUnique<SelectorList>(krys::move(selectorList)));
          return selector;
        }
        case SelectorPseudoClass::Dir:
        case SelectorPseudoClass::State:
        {
          const auto &ident = block.ConsumeIncludingWhitespace();
          if (ident.Type() != TokenType::Ident || !block.IsAtEnd())
          {
            return nullptr;
          }
          selector->SetArgument(CSSOMStringAtom(ident.IdentCodePoints()));
          return selector;
        }
        case SelectorPseudoClass::ActiveViewTransitionType:
        {
          auto typeList = ConsumeCommaSeparatedCustomIdentList(block);
          if (!typeList)
          {
            return nullptr;
          }
          selector->SetArgumentList(krys::move(*typeList));
          return selector;
        }
        default:
        {
          break;
        }
      }
    }

    if (selector->Match() == SelectorMatch::PseudoElement)
    {
      switch (selector->PseudoElement())
      {
        case SelectorPseudoElement::Cue:
        {
          auto selectorList = ConsumeCompoundSelectorList(block);
          if (selectorList.empty() || !block.IsAtEnd())
          {
            return nullptr;
          }
          selector->SetSubSelectors(CreateUnique<SelectorList>(krys::move(selectorList)));
          return selector;
        }
        case SelectorPseudoElement::Highlight:
        {
          const auto &ident = block.ConsumeIncludingWhitespace();
          if (ident.Type() != TokenType::Ident || !block.IsAtEnd())
          {
            return nullptr;
          }
          selector->SetArgumentList(FixedVector<CSSOMStringAtom> {CSSOMStringAtom(ident.IdentCodePoints())});
          return selector;
        }
        case SelectorPseudoElement::ViewTransitionGroup:
        case SelectorPseudoElement::ViewTransitionImagePair:
        case SelectorPseudoElement::ViewTransitionOld:
        case SelectorPseudoElement::ViewTransitionNew:
        {
          List<CSSOMStringAtom> nameAndClasses;

          // Check for implicit universal selector.
          if (IsDelimiter(block.Peek(), U'.'))
          {
            nameAndClasses.push_back(dom::StarAtom());
          }

          // Parse name or explicit universal selector.
          if (nameAndClasses.empty())
          {
            const auto &ident = block.Consume();
            if (ident.Type() == TokenType::Ident && IsValidCustomIdentifier(ident.ValueId()))
            {
              nameAndClasses.push_back(CSSOMStringAtom(ident.IdentCodePoints()));
            }
            else if (IsDelimiter(ident, U'*'))
            {
              nameAndClasses.push_back(dom::StarAtom());
            }
            else
            {
              return nullptr;
            }
          }

          // Parse classes.
          while (!block.IsAtEnd() && block.Peek().Type() != TokenType::Whitespace)
          {
            if (!IsDelimiter(block.Peek(), U'.'))
            {
              return nullptr;
            }
            std::ignore = block.Consume();

            if (block.Peek().Type() != TokenType::Ident)
            {
              return nullptr;
            }
            nameAndClasses.push_back(CSSOMStringAtom(block.Consume().IdentCodePoints()));
          }

          block.DiscardWhitespace();

          if (!block.IsAtEnd())
          {
            return nullptr;
          }

          selector->SetArgumentList(
            FixedVector<CSSOMStringAtom>(nameAndClasses.begin(), nameAndClasses.end()));
          return selector;
        }
        case SelectorPseudoElement::Part:
        {
          List<CSSOMStringAtom> argumentList;
          do
          {
            const auto &ident = block.ConsumeIncludingWhitespace();
            if (ident.Type() != TokenType::Ident)
            {
              return nullptr;
            }
            argumentList.push_back(CSSOMStringAtom(ident.IdentCodePoints()));
          } while (!block.IsAtEnd());
          selector->SetArgumentList(FixedVector<CSSOMStringAtom>(argumentList.begin(), argumentList.end()));
          return selector;
        }
        case SelectorPseudoElement::Slotted:
        {
          auto innerSelector = ConsumeCompoundSelector(block);
          block.DiscardWhitespace();
          if (!innerSelector || !block.IsAtEnd())
          {
            return nullptr;
          }
          MutableSelectorList list;
          list.push_back(krys::move(innerSelector));
          selector->AdoptSelectorVector(krys::move(list));
          return selector;
        }
        default:
        {
          break;
        }
      }
    }

    return nullptr;
  }

  SelectorRelation SelectorParser::ConsumeCombinator(TokenRange &tokens) noexcept
  {
    auto fallbackResult = SelectorRelation::Compounding;
    while (tokens.Peek().Type() == TokenType::Whitespace)
    {
      std::ignore = tokens.Consume();
      fallbackResult = SelectorRelation::Descendant;
    }

    if (tokens.Peek().Type() != TokenType::Delim)
    {
      return fallbackResult;
    }

    char32 delimiter = tokens.Peek().Delimiter();

    if (delimiter == U'+' || delimiter == U'~' || delimiter == U'>')
    {
      std::ignore = tokens.ConsumeIncludingWhitespace();
      if (delimiter == U'+')
      {
        return SelectorRelation::NextSibling;
      }
      if (delimiter == U'~')
      {
        return SelectorRelation::SubsequentSibling;
      }
      return SelectorRelation::Child;
    }

    return fallbackResult;
  }

  SelectorMatch SelectorParser::ConsumeAttributeMatch(TokenRange &tokens) noexcept
  {
    // The tokenizer produces the operators as delimiters; no whitespace is allowed between their two halves.
    const auto &token = tokens.Consume();
    if (IsDelimiter(token, U'='))
    {
      tokens.DiscardWhitespace();
      return SelectorMatch::AttributeEquals;
    }

    auto match = [&]() -> Maybe<SelectorMatch>
    {
      if (token.Type() != TokenType::Delim)
      {
        return {};
      }
      switch (token.Delimiter())
      {
        case U'~':
        {
          return SelectorMatch::AttributeIncludes;
        }
        case U'|':
        {
          return SelectorMatch::AttributeDash;
        }
        case U'^':
        {
          return SelectorMatch::AttributePrefix;
        }
        case U'$':
        {
          return SelectorMatch::AttributeSuffix;
        }
        case U'*':
        {
          return SelectorMatch::AttributeSubstring;
        }
        default:
        {
          return {};
        }
      }
    }();

    if (match && IsDelimiter(tokens.Peek(), U'='))
    {
      std::ignore = tokens.ConsumeIncludingWhitespace();
      return *match;
    }

    _failedParsing = true;
    return SelectorMatch::AttributeEquals;
  }

  IsCaseSensitive SelectorParser::ConsumeAttributeFlags(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != TokenType::Ident)
    {
      return IsCaseSensitive(true);
    }
    const auto &flag = tokens.ConsumeIncludingWhitespace();
    if (text::IsASCIICaselessEqual(flag.IdentCodePoints(), CSSOMStringView {u8"i"}))
    {
      return IsCaseSensitive(false);
    }
    // FIXME: The `s` flag (selectors-4) isn't supported.
    _failedParsing = true;
    return IsCaseSensitive(true);
  }

  namespace
  {
    void AppendInteger(int32 value, CSSOMString &builder) noexcept
    {
      Array<char, 16uz> buffer;
      auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
      krys_debug_assert(result.ec == std::errc {});
      builder.append(reinterpret_cast<const char8 *>(buffer.data()),
                     reinterpret_cast<const char8 *>(result.ptr));
    }

    // <an+b> token sequences have special serialisation rules:
    // https://www.w3.org/TR/css-syntax-3/#serializing-anb
    CSSOMStringAtom SerialiseANPlusB(const Pair<int32, int32> &ab) noexcept
    {
      CSSOMString builder;

      if (!ab.first)
      {
        AppendInteger(ab.second, builder);
        return CSSOMStringAtom(builder);
      }

      if (ab.first == -1)
      {
        builder += u8'-';
      }
      else if (ab.first != 1)
      {
        AppendInteger(ab.first, builder);
      }
      builder += u8'n';

      if (ab.second)
      {
        if (ab.second > 0)
        {
          builder += u8'+';
        }
        AppendInteger(ab.second, builder);
      }

      return CSSOMStringAtom(builder);
    }

    KRYS_NODISCARD Maybe<int32> ParseInteger(CSSOMStringView string) noexcept
    {
      int32 value = 0;
      const auto *begin = reinterpret_cast<const char *>(string.data());
      const auto *end = begin + string.size();
      auto result = std::from_chars(begin, end, value);
      if (result.ec != std::errc {} || result.ptr != end)
      {
        return {};
      }
      return value;
    }

    bool ConsumeANPlusB(TokenRange &tokens, Pair<int32, int32> &result) noexcept
    {
      const auto &token = tokens.Consume();
      if (token.Type() == TokenType::Number && token.NumericValueType() == NumericValueType::Integer)
      {
        result = {0, ClampToInt32(token.NumericValue())};
        return true;
      }
      if (token.Type() == TokenType::Ident)
      {
        if (text::IsASCIICaselessEqual(token.IdentCodePoints(), CSSOMStringView {u8"odd"}))
        {
          result = {2, 1};
          return true;
        }
        if (text::IsASCIICaselessEqual(token.IdentCodePoints(), CSSOMStringView {u8"even"}))
        {
          result = {2, 0};
          return true;
        }
      }

      // The 'n' will end up as part of an ident or dimension. For a valid <an+b>, this will store a string of
      // the form 'n', 'n-', or 'n-123'.
      CSSOMStringView nString;

      if (IsDelimiter(token, U'+') && tokens.Peek().Type() == TokenType::Ident)
      {
        result.first = 1;
        nString = tokens.Consume().IdentCodePoints();
      }
      else if (token.Type() == TokenType::Dimension && token.NumericValueType() == NumericValueType::Integer)
      {
        result.first = ClampToInt32(token.NumericValue());
        nString = token.Unit();
      }
      else if (token.Type() == TokenType::Ident)
      {
        auto value = token.IdentCodePoints();
        if (value[0] == u8'-')
        {
          result.first = -1;
          nString = value.substr(1);
        }
        else
        {
          result.first = 1;
          nString = value;
        }
      }

      tokens.DiscardWhitespace();

      if (nString.empty() || !text::IsASCIICaselessEqual(nString[0], 'n'))
      {
        return false;
      }
      if (nString.size() > 1 && nString[1] != u8'-')
      {
        return false;
      }

      if (nString.size() > 2)
      {
        auto parsedNumber = ParseInteger(nString.substr(1));
        result.second = parsedNumber.value_or(0);
        return parsedNumber.has_value();
      }

      auto sign = nString.size() == 1 ? NumericSignChar::Missing : NumericSignChar::Minus;
      if (sign == NumericSignChar::Missing && tokens.Peek().Type() == TokenType::Delim)
      {
        char32 delimiterSign = tokens.ConsumeIncludingWhitespace().Delimiter();
        if (delimiterSign == U'+')
        {
          sign = NumericSignChar::Plus;
        }
        else if (delimiterSign == U'-')
        {
          sign = NumericSignChar::Minus;
        }
        else
        {
          return false;
        }
      }

      if (sign == NumericSignChar::Missing && tokens.Peek().Type() != TokenType::Number)
      {
        result.second = 0;
        return true;
      }

      const auto &b = tokens.Consume();
      if (b.Type() != TokenType::Number || b.NumericValueType() != NumericValueType::Integer)
      {
        return false;
      }
      if ((b.NumericSign() == NumericSignChar::Missing) == (sign == NumericSignChar::Missing))
      {
        return false;
      }
      result.second = ClampToInt32(b.NumericValue());
      if (sign == NumericSignChar::Minus)
      {
        result.second = -result.second;
      }
      return true;
    }
  }

  const CSSOMStringAtom &SelectorParser::DefaultNamespace() const noexcept
  {
    if (!_styleSheet || _ignoreDefaultNamespace)
    {
      return dom::StarAtom();
    }
    return _styleSheet->DefaultNamespace();
  }

  const CSSOMStringAtom &SelectorParser::DetermineNamespace(const CSSOMStringAtom &prefix) const noexcept
  {
    if (prefix == CSSOMStringAtom::Null())
    {
      return DefaultNamespace();
    }
    if (prefix == CSSOMStringAtom::Empty())
    {
      return CSSOMStringAtom::Empty(); // No namespace. If an element/attribute has a namespace, we won't
                                       // match it.
    }
    if (prefix == dom::StarAtom())
    {
      return dom::StarAtom(); // We'll match any namespace.
    }
    if (!_styleSheet)
    {
      return CSSOMStringAtom::Null(); // Cannot resolve prefix to namespace without a stylesheet, syntax
                                      // error.
    }
    return _styleSheet->NamespaceURIFromPrefix(prefix);
  }

  void SelectorParser::PrependTypeSelectorIfNeeded(const CSSOMStringAtom &namespacePrefix,
                                                   const CSSOMStringAtom &elementName,
                                                   MutableSelector &compoundSelector) noexcept
  {
    bool isShadowDOM = compoundSelector.NeedsImplicitShadowCombinatorForMatching();

    if (elementName == CSSOMStringAtom::Null() && DefaultNamespace() == dom::StarAtom() && !isShadowDOM)
    {
      return;
    }

    CSSOMStringAtom determinedElementName =
      elementName == CSSOMStringAtom::Null() ? dom::StarAtom() : elementName;
    CSSOMStringAtom namespaceURI = DetermineNamespace(namespacePrefix);
    if (namespaceURI == CSSOMStringAtom::Null())
    {
      _failedParsing = true;
      return;
    }
    CSSOMStringAtom determinedPrefix = namespacePrefix;
    if (namespaceURI == DefaultNamespace())
    {
      determinedPrefix = CSSOMStringAtom::Null();
    }
    dom::QualifiedName tag(namespaceURI, determinedPrefix, determinedElementName);

    // *:host never matches, so we can't discard the *, otherwise we can't tell the difference between *:host
    // and just :host.
    //
    // Also, selectors where we use a ShadowPseudo combinator between the element and the pseudo element for
    // matching (custom pseudo elements, ::cue), we need a universal selector to set the combinator (relation)
    // on in the cases where there are no simple selectors preceding the pseudo element.
    bool isHostPseudo = compoundSelector.IsHostPseudoClass();
    if (isHostPseudo && elementName == CSSOMStringAtom::Null() && namespacePrefix == CSSOMStringAtom::Null())
    {
      return;
    }
    if (tag != dom::AnyQualifiedName() || isHostPseudo || isShadowDOM)
    {
      compoundSelector.AppendTagInComplexSelector(tag, determinedPrefix == CSSOMStringAtom::Null()
                                                         && determinedElementName == dom::StarAtom()
                                                         && !isHostPseudo);
    }
  }

  UniquePtr<MutableSelector> SelectorParser::SplitCompoundAtImplicitShadowCrossingCombinator(
    UniquePtr<MutableSelector> compoundSelector, const SelectorParserContext &context) noexcept
  {
    // Complex selectors are represented as a linked list that stores combinator separated compound selectors
    // from right-to-left. Yet, within a single compound selector, stores the simple selectors from
    // left-to-right.
    //
    // ".a.b > div#id" is stored in a complex selector as [div, #id, .a, .b], each element in the list stored
    // with an associated relation (combinator or Compounding).
    //
    // ::cue, ::part(), ::slotted() and user agent parts have an implicit shadow combinator to their left,
    // which really makes for a new compound selector, yet it's consumed by the selector parser as a single
    // compound selector.
    //
    // Example: input#x::details-content -> [ ::details-content, input, #x ]
    //
    auto *splitAfter = compoundSelector.get();
    while (splitAfter->PrecedingComplexSelectorComponent()
           && !splitAfter->PrecedingComplexSelectorComponent()->NeedsImplicitShadowCombinatorForMatching())
    {
      splitAfter = splitAfter->PrecedingComplexSelectorComponent();
    }

    if (!splitAfter || !splitAfter->PrecedingComplexSelectorComponent())
    {
      return compoundSelector;
    }

    const auto *preceding = splitAfter->PrecedingComplexSelectorComponent();

    // ::part() combines with other pseudo elements.
    bool isPart = preceding->Match() == SelectorMatch::PseudoElement
                  && preceding->PseudoElement() == SelectorPseudoElement::Part;

    // ::slotted() combines with other pseudo elements.
    bool isSlotted = preceding->Match() == SelectorMatch::PseudoElement
                     && preceding->PseudoElement() == SelectorPseudoElement::Slotted;

    UniquePtr<MutableSelector> secondCompound;
    if (IsUASheetBehavior(context.Mode) || isPart)
    {
      // FIXME: https://bugs.webkit.org/show_bug.cgi?id=161747
      // We have to recur, since we have rules in media controls like video::a::b. This should not be allowed,
      // and we should remove this recursion once those rules are gone.
      secondCompound =
        SplitCompoundAtImplicitShadowCrossingCombinator(splitAfter->ReleaseFromComplexSelector(), context);
    }
    else
    {
      secondCompound = splitAfter->ReleaseFromComplexSelector();
    }

    auto relation = [&]
    {
      if (isSlotted)
      {
        return SelectorRelation::ShadowSlotted;
      }
      if (isPart)
      {
        return SelectorRelation::ShadowPartDescendant;
      }
      return SelectorRelation::ShadowDescendant;
    }();
    secondCompound->PrependComplexSelectorComponent(relation, krys::move(compoundSelector));
    return secondCompound;
  }

  SelectorList SelectorParser::ResolveNestingParent(const SelectorList &nestedSelectorList,
                                                    const SelectorList *parentResolvedSelectorList,
                                                    bool parentRuleIsScope) noexcept
  {
    MutableSelectorList result;

    auto canInline = [](const Selector &nestingSelector, const SelectorList &list)
    {
      auto hasTagInCompound = [](const Selector &simpleSelector)
      {
        // A compound is organized so that any tag selector is always last.
        return simpleSelector.LastInCompoundSelector()->Match() == SelectorMatch::Type;
      };

      if (list.SelectorCount() != 1)
      {
        // .foo, .bar { & .baz {...} } -> :is(.foo, .bar) .baz {...}
        return false;
      }
      if (ComplexSelectorCanMatchPseudoElement(*list.FirstSelectorComponent()))
      {
        // .foo::before { & {...} } -> :is(.foo::before) {...} (which matches nothing)
        return false;
      }
      if (hasTagInCompound(*list.FirstSelectorComponent()) && hasTagInCompound(nestingSelector))
      {
        // foo { bar& {...} } -> bar:is(foo) {...}
        return false;
      }
      if (!nestingSelector.PrecedingComplexSelectorComponent())
      {
        // .foo .bar { & .baz {...} } -> .foo .bar .baz {...}
        return true;
      }
      bool hasSingleCompound =
        !list.FirstSelectorComponent()->FirstInCompoundSelector()->PrecedingComplexSelectorComponent();
      if (hasSingleCompound)
      {
        // .foo.bar { .baz & {...} } -> .baz .foo.bar {...}
        return true;
      }
      // .foo .bar { .baz & {...} } -> .baz :is(.foo .bar) {...}
      return false;
    };

    auto resolveNestingSelector = [&](const Selector &nestingSelector) -> UniquePtr<MutableSelector>
    {
      krys_debug_assert(nestingSelector.Match() == SelectorMatch::NestingParent);

      if (parentResolvedSelectorList && !parentRuleIsScope)
      {
        if (canInline(nestingSelector, *parentResolvedSelectorList))
        {
          // :is() not needed.
          return CreateUnique<MutableSelector>(*parentResolvedSelectorList->FirstSelectorComponent());
        }
        // General case where we wrap with :is().
        auto isSelector = CreateUnique<MutableSelector>();
        isSelector->SetMatch(SelectorMatch::PseudoClass);
        isSelector->SetPseudoClass(SelectorPseudoClass::Is);
        isSelector->SetSubSelectors(CreateUnique<SelectorList>(*parentResolvedSelectorList));
        return isSelector;
      }

      // Top-level nesting parent selector acts like :scope with zero specificity thanks to :where
      // https://github.com/w3c/csswg-drafts/issues/10196#issuecomment-2161119978
      // Replace by :where(:scope)
      auto scopeSelector = CreateUnique<MutableSelector>();
      scopeSelector->SetMatch(SelectorMatch::PseudoClass);
      scopeSelector->SetPseudoClass(SelectorPseudoClass::Scope);
      MutableSelectorList scopeSelectorList;
      scopeSelectorList.push_back(krys::move(scopeSelector));

      auto whereSelector = CreateUnique<MutableSelector>();
      whereSelector->SetMatch(SelectorMatch::PseudoClass);
      whereSelector->SetPseudoClass(SelectorPseudoClass::Where);
      whereSelector->SetSubSelectors(CreateUnique<SelectorList>(krys::move(scopeSelectorList)));
      return whereSelector;
    };

    auto resolveSimpleSelector = [&](const Selector &simpleSelector) -> UniquePtr<MutableSelector>
    {
      if (simpleSelector.Match() == SelectorMatch::NestingParent)
      {
        return resolveNestingSelector(simpleSelector);
      }

      auto resolvedSelector =
        CreateUnique<MutableSelector>(simpleSelector, MutableSelector::SimpleSelectorTag {});

      if (const auto *subselectorList = simpleSelector.SubSelectors();
          subselectorList && subselectorList->HasExplicitNestingParent())
      {
        // Resolve nested selector lists like :has(&).
        auto resolvedSubselectorList =
          ResolveNestingParent(*subselectorList, parentResolvedSelectorList, parentRuleIsScope);
        resolvedSelector->SetSubSelectors(CreateUnique<SelectorList>(krys::move(resolvedSubselectorList)));
      }
      return resolvedSelector;
    };

    for (const auto &complexSelector : nestedSelectorList)
    {
      MutableSelector *leftmost = nullptr;
      for (const auto *simpleSelector = &complexSelector; simpleSelector;
           simpleSelector = simpleSelector->PrecedingComplexSelectorComponent())
      {
        auto resolvedSimpleSelector = resolveSimpleSelector(*simpleSelector);

        if (!leftmost)
        {
          result.push_back(krys::move(resolvedSimpleSelector));
          leftmost = result.back().get();
        }
        else
        {
          leftmost->SetPrecedingComplexSelectorComponent(krys::move(resolvedSimpleSelector));
        }

        // A nesting selector may resolve to multiple selectors. Find the leftmost one to continue.
        leftmost = leftmost->LeftmostSimpleSelector();
        leftmost->SetRelation(simpleSelector->Relation());
      }
    }

    return SelectorList {krys::move(result)};
  }

  namespace
  {
    KRYS_NODISCARD Maybe<style::PseudoElementIdentifier>
      PseudoElementIdentifierFor(SelectorPseudoElement selectorPseudoElement) noexcept
    {
      auto type = Selector::StylePseudoElementTypeFor(selectorPseudoElement);
      if (!type)
      {
        return {};
      }
      return style::PseudoElementIdentifier {*type};
    }
  }

  // FIXME: It's probably worth investigating if more logic can be shared with
  // SelectorParser::ConsumePseudo(), though note that the requirements are subtly different.
  Pair<bool, Maybe<style::PseudoElementIdentifier>>
    SelectorParser::ParsePseudoElement(CSSOMStringView input, const SelectorParserContext &context) noexcept
  {
    InputStream inputStream {CSSOMString(input)};
    Tokenizer tokenizer(inputStream);
    std::ignore = tokenizer.PumpTokenizer();
    auto range = tokenizer.Tokens();

    if (range.Consume().Type() != TokenType::Colon)
    {
      return {};
    }
    const auto *token = &range.Consume();
    if (token->Type() == TokenType::Ident)
    {
      if (!range.IsAtEnd())
      {
        return {};
      }
      auto pseudoClassOrElement = FindPseudoClassAndCompatibilityElementName(token->IdentCodePoints());
      if (!pseudoClassOrElement.CompatibilityPseudoElement)
      {
        return {};
      }
      krys_debug_assert(Selector::IsPseudoElementEnabled(*pseudoClassOrElement.CompatibilityPseudoElement,
                                                         token->IdentCodePoints(), context));
      return {true, PseudoElementIdentifierFor(*pseudoClassOrElement.CompatibilityPseudoElement)};
    }
    if (token->Type() != TokenType::Colon)
    {
      return {};
    }
    token = &range.Peek();
    if (token->Type() != TokenType::Ident && token->Type() != TokenType::Function)
    {
      return {};
    }
    auto pseudoElement = Selector::ParsePseudoElementName(token->IdentCodePoints(), context);
    if (!pseudoElement)
    {
      return {};
    }
    if (token->Type() == TokenType::Ident)
    {
      std::ignore = range.Consume();
      if (!range.IsAtEnd() || Selector::PseudoElementRequiresArgument(*pseudoElement))
      {
        return {};
      }
      return {true, PseudoElementIdentifierFor(*pseudoElement)};
    }
    krys_debug_assert(token->Type() == TokenType::Function);
    auto block = range.ConsumeBlock();
    if (!range.IsAtEnd())
    {
      return {};
    }
    block.DiscardWhitespace();
    switch (*pseudoElement)
    {
      case SelectorPseudoElement::Highlight:
      {
        const auto &ident = block.ConsumeIncludingWhitespace();
        if (ident.Type() != TokenType::Ident || !block.IsAtEnd())
        {
          return {};
        }
        return {true, style::PseudoElementIdentifier {PseudoElementType::Highlight,
                                                      CSSOMStringAtom(ident.IdentCodePoints())}};
      }
      case SelectorPseudoElement::ViewTransitionGroup:
      case SelectorPseudoElement::ViewTransitionImagePair:
      case SelectorPseudoElement::ViewTransitionOld:
      case SelectorPseudoElement::ViewTransitionNew:
      {
        const auto &ident = block.ConsumeIncludingWhitespace();
        if (ident.Type() != TokenType::Ident || !IsValidCustomIdentifier(ident.ValueId()) || !block.IsAtEnd())
        {
          return {};
        }
        return {true, style::PseudoElementIdentifier {*Selector::StylePseudoElementTypeFor(*pseudoElement),
                                                      CSSOMStringAtom(ident.IdentCodePoints())}};
      }
      default:
      {
        return {};
      }
    }
  }
}
