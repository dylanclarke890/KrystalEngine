#include "Krystal.HTML/CSS/Selectors/CSSSelectorParser.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSNamespaceRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.HTML/CSS/Selectors/CSSSelector.hpp"
#include "Krystal.HTML/CSS/Selectors/MutableCSSSelector.hpp"
#include "Krystal.HTML/CSS/StyleSheetContents.hpp"
#include "Krystal.Lib/Utils/SetForScope.hpp"

namespace Krys::HTML
{
  Maybe<CSSSelectorList> CSSSelectorParser::ParseSelector(CSSOMString &&input,
                                                          const CSSSelectorParserContext &context,
                                                          RawPtr<StyleSheetContents> stylesheet,
                                                          Maybe<NestedContextType> nestedContext) noexcept
  {
    CSSInputStream inputStream(Krys::Move(input));
    CSSTokenizer tokenizer(inputStream);
    if (!tokenizer.PumpTokenizer())
    {
      return Null;
    }
    return ParseSelector(tokenizer.TokenRange(), context, stylesheet, nestedContext);
  }

  Maybe<CSSSelectorList> CSSSelectorParser::ParseSelector(CSSTokenRange tokens,
                                                          const CSSSelectorParserContext &context,
                                                          RawPtr<StyleSheetContents> stylesheet,
                                                          Maybe<NestedContextType> nestedContext) noexcept
  {
    auto result = ParseMutableSelectorList(tokens, context, stylesheet, nestedContext,
                                           IsForgivingSelectorList(false), DisallowPseudoElements(false));

    if (result.empty() || !tokens.IsAtEnd())
    {
      return Null;
    }

    return CSSSelectorList {Krys::Move(result)};
  }

  MutableCSSSelectorList CSSSelectorParser::ParseMutableSelectorList(
    CSSTokenRange &range, const CSSSelectorParserContext &context, RawPtr<StyleSheetContents> stylesheet,
    Maybe<NestedContextType> nestedContext, IsForgivingSelectorList isForgiving,
    DisallowPseudoElements disallowPseudoElements) noexcept
  {
    CSSSelectorParser parser(context, stylesheet, nestedContext, disallowPseudoElements);
    range.DiscardWhitespace();

    auto result = [&]() noexcept -> MutableCSSSelectorList
    {
      if (nestedContext && !isForgiving)
      {
        return parser.ConsumeNestedSelectorList(range);
      }

      if (nestedContext && isForgiving)
      {
        return parser.ConsumeNestedComplexForgivingSelectorList(range);
      }

      if (isForgiving)
      {
        return parser.ConsumeComplexForgivingSelectorList(range);
      }

      return parser.ConsumeComplexSelectorList(range);
    }();

    if (result.empty() || !range.IsAtEnd())
    {
      return {};
    }

    return result;
  }

#pragma region Selector List Parsing

  template <typename ConsumeSelector>
  MutableCSSSelectorList CSSSelectorParser::ConsumeSelectorList(CSSTokenRange &range,
                                                                ConsumeSelector &&consumeSelector) noexcept
  {
    MutableCSSSelectorList selectors;
    auto selector = consumeSelector(range);
    if (selector == nullptr)
    {
      return {};
    }

    selectors.push_back(Krys::Move(selector));
    while (!range.IsAtEnd() && range.Peek().Type() == CSSTokenType::Comma)
    {
      range.Discard();
      range.DiscardWhitespace();

      selector = consumeSelector(range);
      if (selector == nullptr)
      {
        return {};
      }

      selectors.push_back(Krys::Move(selector));
    }

    return selectors;
  }

  MutableCSSSelectorList CSSSelectorParser::ConsumeComplexSelectorList(CSSTokenRange &range) noexcept
  {
    return ConsumeSelectorList(range, [&](CSSTokenRange &range) { return ConsumeComplexSelector(range); });
  }

  MutableCSSSelectorList CSSSelectorParser::ConsumeRelativeSelectorList(CSSTokenRange &range) noexcept
  {
    return ConsumeSelectorList(range,
                               [&](CSSTokenRange &range) { return ConsumeRelativeScopeSelector(range); });
  }

  MutableCSSSelectorList CSSSelectorParser::ConsumeNestedSelectorList(CSSTokenRange &range) noexcept
  {
    return ConsumeSelectorList(range,
                               [&](CSSTokenRange &range) { return ConsumeNestedComplexSelector(range); });
  }

  template <typename ConsumeSelector>
  MutableCSSSelectorList
    CSSSelectorParser::ConsumeForgivingSelectorList(CSSTokenRange &range,
                                                    ConsumeSelector &&consumeSelector) noexcept
  {
    MutableCSSSelectorList selectors;

    auto consumeForgiving = [&]() noexcept -> void
    {
      auto initialRange = range;
      auto unknownSelector = [&]() noexcept -> UniquePtr<MutableCSSSelector>
      {
        auto unknownSelector = CreateUnique<MutableCSSSelector>();
        auto unknownRange = initialRange.RangeUntil(range);

        unknownSelector->SetMatch(SelectorMatch::ForgivingUnknown);

        // If the range contains a nesting selector, we mark this unknown selector as "nest containing" (it
        // will be used during rule set building)
        for (const CSSToken &token : unknownRange)
        {
          if (token.Type() == CSSTokenType::Delim && token.IdentCodePoints() == u8"&")
          {
            unknownSelector->SetMatch(SelectorMatch::ForgivingUnknownNestContaining);
            break;
          }
        }

        // TODO: store the complete range content for serialization.
        // unknownSelector->SetValue(unknownRange.serialize());

        return unknownSelector;
      };

      auto selector = consumeSelector(range);

      // Range is not over and next token is not a comma (means there is more to this selector) so this
      // selector is unknown. Consume until next comma and add the full range as an unknown selector to the
      // selector list.
      if ((!range.IsAtEnd() && range.Peek().Type() != CSSTokenType::Comma) || selector == nullptr)
      {
        while (!range.IsAtEnd() && range.Peek().Type() != CSSTokenType::Comma)
        {
          range.Discard();
        }

        selectors.push_back(unknownSelector());
        return;
      }

      selectors.push_back(Krys::Move(selector));
    };

    consumeForgiving();
    while (!range.IsAtEnd() && range.Peek().Type() == CSSTokenType::Comma)
    {
      range.Discard();
      range.DiscardWhitespace();
      consumeForgiving();
    }

    return selectors;
  }

  MutableCSSSelectorList CSSSelectorParser::ConsumeComplexForgivingSelectorList(CSSTokenRange &range) noexcept
  {
    return ConsumeForgivingSelectorList(range,
                                        [&](CSSTokenRange &range) { return ConsumeComplexSelector(range); });
  }

  MutableCSSSelectorList
    CSSSelectorParser::ConsumeNestedComplexForgivingSelectorList(CSSTokenRange &range) noexcept
  {
    return ConsumeForgivingSelectorList(range, [&](CSSTokenRange &range)
                                        { return ConsumeNestedComplexSelector(range); });
  }

#pragma endregion

#pragma region Simple Selector Parsing

  UniquePtr<MutableCSSSelector> CSSSelectorParser::ConsumeId(CSSTokenRange &range) noexcept
  {
    assert(range.Peek().Type() == CSSTokenType::Hash);
    if (range.Peek().HashType() != HashTokenType::Id)
    {
      return nullptr;
    }

    auto selector = CreateUnique<MutableCSSSelector>();
    selector->SetMatch(SelectorMatch::Id);

    auto &token = range.Consume();
    selector->SetValue(token.IdentCodePoints(), MatchLowercase(_context.Mode == CSSParserMode::HTMLQuirks));
    return selector;
  }

  UniquePtr<MutableCSSSelector> CSSSelectorParser::ConsumeClass(CSSTokenRange &range) noexcept
  {
    assert(range.Peek().Type() == CSSTokenType::Delim);
    assert(range.Peek().IdentCodePoints() == u8".");

    range.Discard();
    if (range.Peek().Type() != CSSTokenType::Ident)
    {
      return nullptr;
    }

    auto selector = CreateUnique<MutableCSSSelector>();
    selector->SetMatch(SelectorMatch::Class);

    auto &token = range.Consume();
    selector->SetValue(token.IdentCodePoints(), MatchLowercase(_context.Mode == CSSParserMode::HTMLQuirks));

    return selector;
  }

  bool CSSSelectorParser::ConsumeName(CSSTokenRange &range, CSSOMStringAtom &name,
                                      CSSOMStringAtom &namespacePrefix) noexcept
  {
    const CSSToken &firstToken = range.Peek();
    if (firstToken.Type() == CSSTokenType::Ident)
    {
      name = firstToken.IdentCodePoints();
      range.Discard();
    }
    else if (firstToken.Type() == CSSTokenType::Delim && firstToken.IdentCodePoints() == StarAtom())
    {
      name = StarAtom();
      range.Discard();
    }
    else if (firstToken.Type() == CSSTokenType::Delim && firstToken.IdentCodePoints() == u8"|")
    {
      // This is an empty namespace, which'll get assigned this value below
      name = CSSOMStringAtom::Empty();
    }
    else
    {
      return false;
    }

    // early exit if this isn't a namespace prefix or it's a dash match (e.g. [foo|=bar])
    if (range.Peek().Type() != CSSTokenType::Delim || range.Peek().IdentCodePoints() != u8"|"
        || (range.Peek(1uz).Type() == CSSTokenType::Delim) && range.Peek(1uz).IdentCodePoints() == u8"=")
    {
      return true;
    }

    namespacePrefix = name;
    if (range.Peek(1uz).Type() == CSSTokenType::Ident)
    {
      range.Discard();
      name = range.Consume().IdentCodePoints();
    }
    else if (range.Peek(1uz).Type() == CSSTokenType::Ident && range.Peek(1).IdentCodePoints() == StarAtom())
    {
      range.Discard();
      range.Discard();
      name = StarAtom();
    }
    else
    {
      name = CSSOMStringAtom::Null();
      namespacePrefix = CSSOMStringAtom::Null();
      return false;
    }

    return true;
  }

  Maybe<SelectorMatch> CSSSelectorParser::ConsumeAttributeMatch(CSSTokenRange &range) noexcept
  {
    assert(range.Peek().Type() == CSSTokenType::Delim);

    auto EqualsFollows = [&range]() noexcept -> bool
    {
      if (range.Peek().Type() == CSSTokenType::Delim && range.Peek().IdentCodePoints() == u8"=")
      {
        range.Discard();
        return true;
      }
      return false;
    };

    const CSSToken &token = range.Consume();
    range.DiscardWhitespace();

    switch (token.IdentCodePoints()[0])
    {
      case u8'=':
      {
        return SelectorMatch::AttributeEquals;
      }
      case u8'~':
      {
        if (EqualsFollows())
        {
          return SelectorMatch::AttributeIncludes;
        }

        return Null;
      }
      case u8'|':
      {
        if (EqualsFollows())
        {
          return SelectorMatch::AttributeDash;
        }

        return Null;
      }
      case u8'^':
      {
        if (EqualsFollows())
        {
          return SelectorMatch::AttributePrefix;
        }

        return Null;
      }
      case u8'$':
      {
        if (EqualsFollows())
        {
          return SelectorMatch::AttributeSuffix;
        }
        return Null;
      }
      case u8'*':
      {
        if (EqualsFollows())
        {
          return SelectorMatch::AttributeSubstring;
        }
        return Null;
      }
    }

    return Null;
  }

  Maybe<IsCaseSensitive> CSSSelectorParser::ConsumeAttributeCaseSensitiveFlag(CSSTokenRange &range) noexcept
  {
    if (range.Peek().Type() != CSSTokenType::Ident)
    {
      return IsCaseSensitive(true);
    }

    const CSSToken &flag = range.Consume();
    range.DiscardWhitespace();

    if (Krys::Text::ASCIICaseInsensitiveMatch(flag.IdentCodePoints(), u8"i"))
    {
      return IsCaseSensitive(false);
    }

    if (Krys::Text::ASCIICaseInsensitiveMatch(flag.IdentCodePoints(), u8"s"))
    {
      return IsCaseSensitive(true);
    }

    return Null;
  }

  UniquePtr<MutableCSSSelector> CSSSelectorParser::ConsumeAttribute(CSSTokenRange &range) noexcept
  {
    assert(range.Peek().Type() == CSSTokenType::OpenSquare);

    CSSTokenRange block = range.ConsumeBlock();
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

    QualifiedName qualifiedName =
      namespacePrefix == CSSOMStringAtom::Null()
        ? QualifiedName(CSSOMStringAtom::Null(), CSSOMStringAtom::Null(), attributeName)
        : QualifiedName(namespaceURI, namespacePrefix, attributeName);

    auto selector = CreateUnique<MutableCSSSelector>();

    if (block.IsAtEnd())
    {
      selector->SetAttribute(qualifiedName, IsCaseSensitive(true));
      selector->SetMatch(SelectorMatch::AttributeExists);
      return selector;
    }

    auto match = ConsumeAttributeMatch(block);
    if (!match.has_value())
    {
      return nullptr;
    }
    selector->SetMatch(*match);

    const CSSToken &attributeValue = block.Consume();
    block.DiscardWhitespace();

    if (attributeValue.Type() != CSSTokenType::Ident && attributeValue.Type() != CSSTokenType::String)
    {
      return nullptr;
    }

    selector->SetValue(attributeValue.IdentCodePoints(), MatchLowercase(false));

    auto flag = ConsumeAttributeCaseSensitiveFlag(block);
    if (!flag.has_value())
    {
      return nullptr;
    }
    selector->SetAttribute(qualifiedName, *flag);

    if (!block.IsAtEnd())
    {
      return nullptr;
    }

    return selector;
  }

  UniquePtr<MutableCSSSelector> CSSSelectorParser::ConsumeNesting(CSSTokenRange &range) noexcept
  {
    assert(range.Peek().Type() == CSSTokenType::Delim);
    assert(range.Peek().IdentCodePoints() == u8"&");
    range.Discard();

    auto selector = CreateUnique<MutableCSSSelector>();
    selector->SetMatch(SelectorMatch::NestingParent);

    return selector;
  }

  UniquePtr<MutableCSSSelector> CSSSelectorParser::ConsumePseudo(CSSTokenRange &range) noexcept
  {
    return UniquePtr<MutableCSSSelector>();
  }

  UniquePtr<MutableCSSSelector> CSSSelectorParser::ConsumeSimpleSelector(CSSTokenRange &range) noexcept
  {
    const CSSToken &token = range.Peek();
    UniquePtr<MutableCSSSelector> selector = [&]() noexcept -> UniquePtr<MutableCSSSelector>
    {
      switch (token.Type())
      {
        case CSSTokenType::Hash:
        {
          return ConsumeId(range);
        }
        case CSSTokenType::Delim:
        {
          if (token.IdentCodePoints() == u8".")
          {
            return ConsumeClass(range);
          }

          if (token.IdentCodePoints() == u8"&")
          {
            return ConsumeNesting(range);
          }

          break;
        }
        case CSSTokenType::OpenSquare:
        {
          return ConsumeAttribute(range);
        }
        case CSSTokenType::Colon:
        {
          return ConsumePseudo(range);
        }
      }

      return nullptr;
    }();

    if (selector == nullptr)
    {
      return nullptr;
    }

    if (_precedingPseudoElement != nullptr)
    {
      if (!IsSimpleSelectorValidAfterPseudoElement(*selector, *_precedingPseudoElement))
      {
        return nullptr;
      }
    }

    return selector;
  }

  bool CSSSelectorParser::IsSimpleSelectorValidAfterPseudoElement(
    const MutableCSSSelector &simpleSelector, const MutableCSSSelector &compoundPseudoElement) const noexcept
  {
    // TODO: Implement the logic to determine if a simple selector is valid after a pseudo-element.
    assert(false);
    return false;
  }

#pragma endregion

  UniquePtr<MutableCSSSelector> CSSSelectorParser::ConsumeCompoundSelector(CSSTokenRange &range) noexcept
  {
    assert(_precedingPseudoElement == nullptr || _disallowPseudoElements);

    CSSOMStringAtom namespacePrefix = CSSOMStringAtom::Null();
    CSSOMStringAtom elementName = CSSOMStringAtom::Null();
    const bool hasName = ConsumeName(range, elementName, namespacePrefix);

    UniquePtr<MutableCSSSelector> compoundSelector;
    if (!hasName)
    {
      compoundSelector = ConsumeSimpleSelector(range);
      if (compoundSelector == nullptr)
      {
        return nullptr;
      }
      if (compoundSelector->Match() == SelectorMatch::PseudoElement)
      {
        _precedingPseudoElement = compoundSelector.get();
      }
    }

    while (auto simpleSelector = ConsumeSimpleSelector(range))
    {
      if (simpleSelector->Match() == SelectorMatch::PseudoElement)
      {
        _precedingPseudoElement = simpleSelector.get();
      }

      if (compoundSelector != nullptr)
      {
        compoundSelector->PrependComplexSelectorComponent(SelectorRelation::Compounding,
                                                          Krys::Move(simpleSelector));
      }
      else
      {
        compoundSelector = Krys::Move(simpleSelector);
      }
    }

    if (!_disallowPseudoElements)
    {
      _precedingPseudoElement = nullptr;
    }

    // While inside a nested selector like :is(), the default namespace shall be ignored when [1]:
    // - The compound selector represents the subject [2], and
    // - The compound selector does not contain a type/universal selector.
    // @see [1]: https://drafts.csswg.org/selectors/#matches
    // @see [2]: https://drafts.csswg.org/selectors/#selector-subject
    SetForScope ignoreDefaultNamespace(_ignoreDefaultNamespace, _resistDefaultNamespace && !hasName
                                                                  && IsAtEndIgnoringWhitespace(range));
    if (compoundSelector == nullptr)
    {
      CSSOMStringAtom namespaceURI = DetermineNamespace(namespacePrefix);
      if (namespaceURI == CSSOMStringAtom::Null())
      {
        return nullptr;
      }

      if (namespaceURI == DefaultNamespace())
      {
        namespacePrefix = CSSOMStringAtom::Null();
      }

      return CreateUnique<MutableCSSSelector>(QualifiedName(namespaceURI, namespacePrefix, elementName));
    }

    if (PrependTypeSelectorIfNeeded(namespacePrefix, elementName, *compoundSelector) == Null)
    {
      return nullptr;
    }

    return compoundSelector;
  }

  Maybe<bool> CSSSelectorParser::PrependTypeSelectorIfNeeded(const CSSOMStringAtom &namespacePrefix,
                                                             const CSSOMStringAtom &elementName,
                                                             MutableCSSSelector &compoundSelector) noexcept
  {
    bool isShadowDOM = compoundSelector.NeedsImplicitShadowCombinatorForMatching();

    if (elementName == CSSOMStringAtom::Null() && DefaultNamespace() == StarAtom() && !isShadowDOM)
    {
      return false;
    }

    CSSOMStringAtom determinedElementName = elementName == CSSOMStringAtom::Null() ? StarAtom() : elementName;

    CSSOMStringAtom namespaceURI = DetermineNamespace(namespacePrefix);
    if (namespaceURI == CSSOMStringAtom::Null())
    {
      return Null; // Invalid namespace prefix, cannot determine namespace URI
    }

    CSSOMStringAtom determinedPrefix = namespacePrefix;
    if (namespaceURI == DefaultNamespace())
    {
      determinedPrefix = CSSOMStringAtom::Null();
    }

    QualifiedName tag(namespaceURI, determinedPrefix, determinedElementName);

    // *:host never matches, so we can't discard the *,
    // otherwise we can't tell the difference between *:host and just :host.
    //
    // Also, selectors where we use a ShadowPseudo combinator between the
    // element and the pseudo element for matching (custom pseudo elements,
    // ::cue), we need a universal selector to set the combinator
    // (relation) on in the cases where there are no simple selectors preceding
    // the pseudo element.
    bool isHostPseudo = compoundSelector.Match() == SelectorMatch::PseudoClass
                        && compoundSelector.PseudoClass() == PseudoClassId::Host;
    if (isHostPseudo && elementName == CSSOMStringAtom::Null() && namespacePrefix == CSSOMStringAtom::Null())
    {
      return false;
    }

    if (tag != AnyQualifiedName() || isHostPseudo || isShadowDOM)
    {
      compoundSelector.AppendTagInComplexSelector(tag, determinedPrefix == CSSOMStringAtom::Null()
                                                         && determinedElementName == StarAtom()
                                                         && !isHostPseudo);
    }

    return true;
  }

  SelectorRelation CSSSelectorParser::ConsumeCombinator(CSSTokenRange &range) noexcept
  {
    auto fallbackResult = SelectorRelation::Compounding;
    while (range.Peek().Type() == CSSTokenType::Whitespace)
    {
      range.Discard();
      fallbackResult = SelectorRelation::Descendant;
    }

    if (range.Peek().Type() != CSSTokenType::Delim)
    {
      return fallbackResult;
    }

    auto delim = range.Peek().IdentCodePoints()[0];

    if (delim == u8'+' || delim == u8'~' || delim == u8'>')
    {
      range.Discard();
      range.DiscardWhitespace();

      if (delim == u8'+')
      {
        return SelectorRelation::NextSibling;
      }

      if (delim == u8'~')
      {
        return SelectorRelation::SubsequentSibling;
      }

      return SelectorRelation::Child;
    }

    return fallbackResult;
  }

  UniquePtr<MutableCSSSelector> CSSSelectorParser::ConsumeComplexSelector(CSSTokenRange &range) noexcept
  {
    auto HasPseudoElementForRightmostCompound = [](const MutableCSSSelector &selector) noexcept
    {
      return selector.Match() == SelectorMatch::PseudoElement;
    };

    auto selector = ConsumeCompoundSelector(range);
    if (selector == nullptr)
    {
      return nullptr;
    }

    bool hasPseudoElementForRightmostCompound = false;
    for (auto *simple = selector.get(); simple && !hasPseudoElementForRightmostCompound;
         simple = simple->PrecedingComplexSelectorComponent())
    {
      hasPseudoElementForRightmostCompound = HasPseudoElementForRightmostCompound(*simple);
    }

    while (true)
    {
      auto combinator = ConsumeCombinator(range);
      if (combinator == SelectorRelation::Compounding)
      {
        break;
      }

      auto nextSelector = ConsumeCompoundSelector(range);
      if (nextSelector == nullptr)
      {
        return combinator == SelectorRelation::Descendant ? Krys::Move(selector) : nullptr;
      }

      if (hasPseudoElementForRightmostCompound)
      {
        return nullptr;
      }

      auto *end = nextSelector.get();
      hasPseudoElementForRightmostCompound |= HasPseudoElementForRightmostCompound(*end);
      while (auto *next = end->PrecedingComplexSelectorComponent())
      {
        end = next;
        hasPseudoElementForRightmostCompound |= HasPseudoElementForRightmostCompound(*end);
      }

      end->SetRelation(combinator);
      end->SetPrecedingComplexSelectorComponent(Krys::Move(selector));

      selector = Krys::Move(nextSelector);
    }

    return selector;
  }

  UniquePtr<MutableCSSSelector> CSSSelectorParser::ConsumeNestedComplexSelector(CSSTokenRange &range) noexcept
  {
    auto selector = ConsumeComplexSelector(range);
    if (selector != nullptr)
    {
      return selector;
    }

    selector = ConsumeRelativeNestedSelector(range);
    if (selector != nullptr)
    {
      return selector;
    }

    return nullptr;
  }

  UniquePtr<MutableCSSSelector> CSSSelectorParser::ConsumeRelativeScopeSelector(CSSTokenRange &range) noexcept
  {
    auto scopeCombinator = ConsumeCombinator(range);
    if (scopeCombinator == SelectorRelation::Compounding)
    {
      scopeCombinator = SelectorRelation::Descendant;
    }

    auto selector = ConsumeComplexSelector(range);
    if (selector == nullptr)
    {
      return nullptr;
    }

    auto *end = selector.get();
    while (auto *next = end->PrecedingComplexSelectorComponent())
    {
      end = next;
    }

    auto scopeSelector = CreateUnique<MutableCSSSelector>();
    scopeSelector->SetMatch(SelectorMatch::HasScope);

    end->SetRelation(scopeCombinator);
    end->SetPrecedingComplexSelectorComponent(Krys::Move(scopeSelector));

    return selector;
  }

  UniquePtr<MutableCSSSelector>
    CSSSelectorParser::ConsumeRelativeNestedSelector(CSSTokenRange &range) noexcept
  {
    auto scopeCombinator = ConsumeCombinator(range);

    // Nesting should only work with ~ > + combinators in this function.
    // The descendant combinator is handled in another code path.
    if (scopeCombinator != SelectorRelation::NextSibling
        && scopeCombinator != SelectorRelation::SubsequentSibling
        && scopeCombinator != SelectorRelation::Child)
    {
      return nullptr;
    }

    auto selector = ConsumeComplexSelector(range);
    if (selector == nullptr)
    {
      return nullptr;
    }

    auto *last = selector->LeftmostSimpleSelector();
    last->SetRelation(scopeCombinator);

    return selector;
  }

#pragma region Namespaces

  const CSSOMStringAtom &CSSSelectorParser::DetermineNamespace(const CSSOMStringAtom &prefix) const noexcept
  {
    if (prefix == CSSOMStringAtom::Null())
    {
      return DefaultNamespace();
    }

    if (prefix == CSSOMStringAtom::Empty())
    {
      // No namespace. If an element/attribute has a namespace, we won't match it.
      return CSSOMStringAtom::Empty();
    }

    if (prefix == StarAtom())
    {
      // We'll match any namespace.
      return StarAtom();
    }

    if (_stylesheet == nullptr)
    {
      // Cannot resolve prefix to namespace without a stylesheet, syntax error.
      return CSSOMStringAtom::Null();
    }

    return _stylesheet->NamespaceForPrefix(prefix);
  }

  const CSSOMStringAtom &CSSSelectorParser::DefaultNamespace() const noexcept
  {
    if (_stylesheet == nullptr || _ignoreDefaultNamespace)
    {
      return StarAtom();
    }

    return _stylesheet->DefaultNamespace();
  }

  bool CSSSelectorParser::IsAtEndIgnoringWhitespace(CSSTokenRange range) const noexcept
  {
    range.DiscardWhitespace();
    return range.IsAtEnd();
  }

#pragma endregion
}