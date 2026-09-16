#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSNamespaceRule.hpp"
#include "Krystal.Booey/CSS/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/MediaList.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Booey/CSS/Selectors/MutableSelector.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo::css
{
  Maybe<SelectorList>
    SelectorParser::ParseSelector(CSSOMString &&input, const SelectorParserContext &context,
                                     StyleSheetContents *stylesheet,
                                     Maybe<NestedParserContextType> nestedContext) noexcept
  {
    InputStream inputStream(krys::move(input));
    Tokenizer tokenizer(inputStream);
    if (!tokenizer.PumpTokenizer())
    {
      return null;
    }
    return ParseSelector(tokenizer.Tokens(), context, stylesheet, nestedContext);
  }

  Maybe<SelectorList>
    SelectorParser::ParseSelector(TokenRange tokens, const SelectorParserContext &context,
                                     StyleSheetContents *stylesheet,
                                     Maybe<NestedParserContextType> nestedContext) noexcept
  {
    auto result = ParseMutableSelectorList(tokens, context, stylesheet, nestedContext,
                                           IsForgivingSelectorList(false), DisallowPseudoElements(false));

    if (result.empty() || !tokens.IsAtEnd())
    {
      return null;
    }

    return SelectorList {krys::move(result)};
  }

  MutableSelectorList SelectorParser::ParseMutableSelectorList(
    TokenRange &tokens, const SelectorParserContext &context, StyleSheetContents *stylesheet,
    Maybe<NestedParserContextType> nestedContext, IsForgivingSelectorList isForgiving,
    DisallowPseudoElements disallowPseudoElements) noexcept
  {
    SelectorParser parser(context, stylesheet, nestedContext, disallowPseudoElements);
    tokens.DiscardWhitespace();

    auto result = [&]() noexcept -> MutableSelectorList
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
    }();

    if (result.empty() || !tokens.IsAtEnd())
    {
      return {};
    }

    return result;
  }

#pragma region Selector List Parsing

  template <typename ConsumeSelector>
  MutableSelectorList SelectorParser::ConsumeSelectorList(TokenRange &tokens,
                                                                ConsumeSelector &&consumeSelector) noexcept
  {
    MutableSelectorList selectors;
    auto selector = consumeSelector(tokens);
    if (selector == nullptr)
    {
      return {};
    }

    selectors.push_back(krys::move(selector));
    while (!tokens.IsAtEnd() && tokens.Peek().Type() == TokenType::Comma)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();

      selector = consumeSelector(tokens);
      if (selector == nullptr)
      {
        return {};
      }

      selectors.push_back(krys::move(selector));
    }

    return selectors;
  }

  MutableSelectorList SelectorParser::ConsumeComplexSelectorList(TokenRange &tokens) noexcept
  {
    return ConsumeSelectorList(tokens, [&](TokenRange &tokens) { return ConsumeComplexSelector(tokens); });
  }

  MutableSelectorList SelectorParser::ConsumeRelativeSelectorList(TokenRange &tokens) noexcept
  {
    return ConsumeSelectorList(tokens,
                               [&](TokenRange &tokens) { return ConsumeRelativeScopeSelector(tokens); });
  }

  MutableSelectorList SelectorParser::ConsumeNestedSelectorList(TokenRange &tokens) noexcept
  {
    return ConsumeSelectorList(tokens,
                               [&](TokenRange &tokens) { return ConsumeNestedComplexSelector(tokens); });
  }

  template <typename ConsumeSelector>
  MutableSelectorList
    SelectorParser::ConsumeForgivingSelectorList(TokenRange &tokens,
                                                    ConsumeSelector &&consumeSelector) noexcept
  {
    MutableSelectorList selectors;

    auto consumeForgiving = [&]() noexcept -> void
    {
      auto initialRange = tokens;
      auto unknownSelector = [&]() noexcept -> UniquePtr<MutableSelector>
      {
        auto unknownSelector = CreateUnique<MutableSelector>();
        auto unknownRange = initialRange.RangeUntil(tokens);

        unknownSelector->SetMatch(SelectorMatch::ForgivingUnknown);

        // If the tokens contains a nesting selector, we mark this unknown selector as "nest containing" (it
        // will be used during rule set building)
        for (const Token &token : unknownRange)
        {
          if (token.Type() == TokenType::Delim && token.IdentCodePoints() == u8"&")
          {
            unknownSelector->SetMatch(SelectorMatch::ForgivingUnknownNestContaining);
            break;
          }
        }

        // TODO: store the complete tokens content for serialization.
        // unknownSelector->SetValue(unknownRange.serialize());

        return unknownSelector;
      };

      auto selector = consumeSelector(tokens);

      // Range is not over and next token is not a comma (means there is more to this selector) so this
      // selector is unknown. Consume until next comma and add the full tokens as an unknown selector to the
      // selector list.
      if ((!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Comma) || selector == nullptr)
      {
        while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Comma)
        {
          tokens.Discard();
        }

        selectors.push_back(unknownSelector());
        return;
      }

      selectors.push_back(krys::move(selector));
    };

    consumeForgiving();
    while (!tokens.IsAtEnd() && tokens.Peek().Type() == TokenType::Comma)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();
      consumeForgiving();
    }

    return selectors;
  }

  MutableSelectorList SelectorParser::ConsumeComplexForgivingSelectorList(TokenRange &tokens) noexcept
  {
    return ConsumeForgivingSelectorList(tokens,
                                        [&](TokenRange &tokens) { return ConsumeComplexSelector(tokens); });
  }

  MutableSelectorList
    SelectorParser::ConsumeNestedComplexForgivingSelectorList(TokenRange &tokens) noexcept
  {
    return ConsumeForgivingSelectorList(tokens, [&](TokenRange &tokens)
                                        { return ConsumeNestedComplexSelector(tokens); });
  }

#pragma endregion

#pragma region Simple Selector Parsing

  UniquePtr<MutableSelector> SelectorParser::ConsumeId(TokenRange &tokens) noexcept
  {
    assert(tokens.Peek().Type() == TokenType::Hash);
    if (tokens.Peek().HashType() != HashTokenType::Id)
    {
      return nullptr;
    }

    auto selector = CreateUnique<MutableSelector>();
    selector->SetMatch(SelectorMatch::Id);

    auto &token = tokens.Consume();
    selector->SetValue(token.IdentCodePoints(), MatchLowercase(_context.Mode == ParserMode::HTMLQuirks));
    return selector;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeClass(TokenRange &tokens) noexcept
  {
    assert(tokens.Peek().Type() == TokenType::Delim);
    assert(tokens.Peek().IdentCodePoints() == u8".");

    tokens.Discard();
    if (tokens.Peek().Type() != TokenType::Ident)
    {
      return nullptr;
    }

    auto selector = CreateUnique<MutableSelector>();
    selector->SetMatch(SelectorMatch::Class);

    auto &token = tokens.Consume();
    selector->SetValue(token.IdentCodePoints(), MatchLowercase(_context.Mode == ParserMode::HTMLQuirks));

    return selector;
  }

  bool SelectorParser::ConsumeName(TokenRange &tokens, CSSOMStringAtom &name,
                                      CSSOMStringAtom &namespacePrefix) noexcept
  {
    const Token &firstToken = tokens.Peek();
    if (firstToken.Type() == TokenType::Ident)
    {
      name = firstToken.IdentCodePoints();
      tokens.Discard();
    }
    else if (firstToken.Type() == TokenType::Delim && firstToken.IdentCodePoints() == dom::StarAtom())
    {
      name = dom::StarAtom();
      tokens.Discard();
    }
    else if (firstToken.Type() == TokenType::Delim && firstToken.IdentCodePoints() == u8"|")
    {
      // This is an empty namespace, which'll get assigned this value below
      name = CSSOMStringAtom::Empty();
    }
    else
    {
      return false;
    }

    // early exit if this isn't a namespace prefix or it's a dash match (e.g. [foo|=bar])
    if (tokens.Peek().Type() != TokenType::Delim || tokens.Peek().IdentCodePoints() != u8"|"
        || (tokens.Peek(1uz).Type() == TokenType::Delim) && tokens.Peek(1uz).IdentCodePoints() == u8"=")
    {
      return true;
    }

    namespacePrefix = name;
    if (tokens.Peek(1uz).Type() == TokenType::Ident)
    {
      tokens.Discard();
      name = tokens.Consume().IdentCodePoints();
    }
    else if (tokens.Peek(1uz).Type() == TokenType::Ident
             && tokens.Peek(1).IdentCodePoints() == dom::StarAtom())
    {
      tokens.Discard();
      tokens.Discard();
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

  Maybe<SelectorMatch> SelectorParser::ConsumeAttributeMatch(TokenRange &tokens) noexcept
  {
    assert(tokens.Peek().Type() == TokenType::Delim);

    auto EqualsFollows = [&tokens]() noexcept -> bool
    {
      if (tokens.Peek().Type() == TokenType::Delim && tokens.Peek().IdentCodePoints() == u8"=")
      {
        tokens.Discard();
        return true;
      }
      return false;
    };

    const Token &token = tokens.Consume();
    tokens.DiscardWhitespace();

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

        return null;
      }
      case u8'|':
      {
        if (EqualsFollows())
        {
          return SelectorMatch::AttributeDash;
        }

        return null;
      }
      case u8'^':
      {
        if (EqualsFollows())
        {
          return SelectorMatch::AttributePrefix;
        }

        return null;
      }
      case u8'$':
      {
        if (EqualsFollows())
        {
          return SelectorMatch::AttributeSuffix;
        }
        return null;
      }
      case u8'*':
      {
        if (EqualsFollows())
        {
          return SelectorMatch::AttributeSubstring;
        }
        return null;
      }
    }

    return null;
  }

  Maybe<IsCaseSensitive> SelectorParser::ConsumeAttributeCaseSensitiveFlag(TokenRange &tokens) noexcept
  {
    if (tokens.Peek().Type() != TokenType::Ident)
    {
      return IsCaseSensitive(true);
    }

    const Token &flag = tokens.Consume();
    tokens.DiscardWhitespace();

    if (krys::text::IsASCIICaselessEqual(flag.IdentCodePoints(), utf8_stringview(u8"i")))
    {
      return IsCaseSensitive(false);
    }

    if (krys::text::IsASCIICaselessEqual(flag.IdentCodePoints(), utf8_stringview(u8"s")))
    {
      return IsCaseSensitive(true);
    }

    return null;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeAttribute(TokenRange &tokens) noexcept
  {
    assert(tokens.Peek().Type() == TokenType::OpenSquare);

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

    auto match = ConsumeAttributeMatch(block);
    if (!match.has_value())
    {
      return nullptr;
    }
    selector->SetMatch(*match);

    const Token &attributeValue = block.Consume();
    block.DiscardWhitespace();

    if (attributeValue.Type() != TokenType::Ident && attributeValue.Type() != TokenType::String)
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

  UniquePtr<MutableSelector> SelectorParser::ConsumeNesting(TokenRange &tokens) noexcept
  {
    assert(tokens.Peek().Type() == TokenType::Delim);
    assert(tokens.Peek().IdentCodePoints() == u8"&");
    tokens.Discard();

    auto selector = CreateUnique<MutableSelector>();
    selector->SetMatch(SelectorMatch::NestingParent);

    return selector;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumePseudo(TokenRange &tokens) noexcept
  {
    return UniquePtr<MutableSelector>();
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeSimpleSelector(TokenRange &tokens) noexcept
  {
    const Token &token = tokens.Peek();
    UniquePtr<MutableSelector> selector = [&]() noexcept -> UniquePtr<MutableSelector>
    {
      switch (token.Type())
      {
        case TokenType::Hash:
        {
          return ConsumeId(tokens);
        }
        case TokenType::Delim:
        {
          if (token.IdentCodePoints() == u8".")
          {
            return ConsumeClass(tokens);
          }

          if (token.IdentCodePoints() == u8"&")
          {
            return ConsumeNesting(tokens);
          }

          break;
        }
        case TokenType::OpenSquare:
        {
          return ConsumeAttribute(tokens);
        }
        case TokenType::Colon:
        {
          return ConsumePseudo(tokens);
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

  bool SelectorParser::IsSimpleSelectorValidAfterPseudoElement(
    const MutableSelector &simpleSelector, const MutableSelector &compoundPseudoElement) const noexcept
  {
    // TODO: Implement the logic to determine if a simple selector is valid after a pseudo-element.
    assert(false);
    return false;
  }

#pragma endregion

  UniquePtr<MutableSelector> SelectorParser::ConsumeCompoundSelector(TokenRange &tokens) noexcept
  {
    assert(_precedingPseudoElement == nullptr || _disallowPseudoElements);

    CSSOMStringAtom namespacePrefix = CSSOMStringAtom::Null();
    CSSOMStringAtom elementName = CSSOMStringAtom::Null();
    const bool hasName = ConsumeName(tokens, elementName, namespacePrefix);

    UniquePtr<MutableSelector> compoundSelector;
    if (!hasName)
    {
      compoundSelector = ConsumeSimpleSelector(tokens);
      if (compoundSelector == nullptr)
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

      if (compoundSelector != nullptr)
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
    // - The compound selector represents the subject [2], and
    // - The compound selector does not contain a type/universal selector.
    // @see [1]: https://drafts.csswg.org/selectors/#matches
    // @see [2]: https://drafts.csswg.org/selectors/#selector-subject
    SetForScope ignoreDefaultNamespace(_ignoreDefaultNamespace, _resistDefaultNamespace && !hasName
                                                                  && IsAtEndIgnoringWhitespace(tokens));
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

      return CreateUnique<MutableSelector>(dom::QualifiedName(namespaceURI, namespacePrefix, elementName));
    }

    if (PrependTypeSelectorIfNeeded(namespacePrefix, elementName, *compoundSelector) == null)
    {
      return nullptr;
    }

    return compoundSelector;
  }

  Maybe<bool> SelectorParser::PrependTypeSelectorIfNeeded(const CSSOMStringAtom &namespacePrefix,
                                                             const CSSOMStringAtom &elementName,
                                                             MutableSelector &compoundSelector) noexcept
  {
    bool isShadowDOM = compoundSelector.NeedsImplicitShadowCombinatorForMatching();

    if (elementName == CSSOMStringAtom::Null() && DefaultNamespace() == dom::StarAtom() && !isShadowDOM)
    {
      return false;
    }

    CSSOMStringAtom determinedElementName =
      elementName == CSSOMStringAtom::Null() ? dom::StarAtom() : elementName;

    CSSOMStringAtom namespaceURI = DetermineNamespace(namespacePrefix);
    if (namespaceURI == CSSOMStringAtom::Null())
    {
      return null; // Invalid namespace prefix, cannot determine namespace URI
    }

    CSSOMStringAtom determinedPrefix = namespacePrefix;
    if (namespaceURI == DefaultNamespace())
    {
      determinedPrefix = CSSOMStringAtom::Null();
    }

    dom::QualifiedName tag(namespaceURI, determinedPrefix, determinedElementName);

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

    if (tag != dom::AnyQualifiedName() || isHostPseudo || isShadowDOM)
    {
      compoundSelector.AppendTagInComplexSelector(tag, determinedPrefix == CSSOMStringAtom::Null()
                                                         && determinedElementName == dom::StarAtom()
                                                         && !isHostPseudo);
    }

    return true;
  }

  SelectorRelation SelectorParser::ConsumeCombinator(TokenRange &tokens) noexcept
  {
    auto fallbackResult = SelectorRelation::Compounding;
    while (tokens.Peek().Type() == TokenType::Whitespace)
    {
      tokens.Discard();
      fallbackResult = SelectorRelation::Descendant;
    }

    if (tokens.Peek().Type() != TokenType::Delim)
    {
      return fallbackResult;
    }

    auto delim = tokens.Peek().IdentCodePoints()[0];

    if (delim == u8'+' || delim == u8'~' || delim == u8'>')
    {
      tokens.Discard();
      tokens.DiscardWhitespace();

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

  UniquePtr<MutableSelector> SelectorParser::ConsumeComplexSelector(TokenRange &tokens) noexcept
  {
    auto HasPseudoElementForRightmostCompound = [](const MutableSelector &selector) noexcept
    {
      return selector.Match() == SelectorMatch::PseudoElement;
    };

    auto selector = ConsumeCompoundSelector(tokens);
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
      auto combinator = ConsumeCombinator(tokens);
      if (combinator == SelectorRelation::Compounding)
      {
        break;
      }

      auto nextSelector = ConsumeCompoundSelector(tokens);
      if (nextSelector == nullptr)
      {
        return combinator == SelectorRelation::Descendant ? krys::move(selector) : nullptr;
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
      end->SetPrecedingComplexSelectorComponent(krys::move(selector));

      selector = krys::move(nextSelector);
    }

    return selector;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeNestedComplexSelector(TokenRange &tokens) noexcept
  {
    auto selector = ConsumeComplexSelector(tokens);
    if (selector != nullptr)
    {
      return selector;
    }

    selector = ConsumeRelativeNestedSelector(tokens);
    if (selector != nullptr)
    {
      return selector;
    }

    return nullptr;
  }

  UniquePtr<MutableSelector> SelectorParser::ConsumeRelativeScopeSelector(TokenRange &tokens) noexcept
  {
    auto scopeCombinator = ConsumeCombinator(tokens);
    if (scopeCombinator == SelectorRelation::Compounding)
    {
      scopeCombinator = SelectorRelation::Descendant;
    }

    auto selector = ConsumeComplexSelector(tokens);
    if (selector == nullptr)
    {
      return nullptr;
    }

    auto *end = selector.get();
    while (auto *next = end->PrecedingComplexSelectorComponent())
    {
      end = next;
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
    if (selector == nullptr)
    {
      return nullptr;
    }

    auto *last = selector->LeftmostSimpleSelector();
    last->SetRelation(scopeCombinator);

    return selector;
  }

#pragma region Namespaces

  const CSSOMStringAtom &SelectorParser::DetermineNamespace(const CSSOMStringAtom &prefix) const noexcept
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

    if (prefix == dom::StarAtom())
    {
      // We'll match any namespace.
      return dom::StarAtom();
    }

    if (_stylesheet == nullptr)
    {
      // Cannot resolve prefix to namespace without a stylesheet, syntax error.
      return CSSOMStringAtom::Null();
    }

    return _stylesheet->NamespaceForPrefix(prefix);
  }

  const CSSOMStringAtom &SelectorParser::DefaultNamespace() const noexcept
  {
    if (_stylesheet == nullptr || _ignoreDefaultNamespace)
    {
      return dom::StarAtom();
    }

    return _stylesheet->DefaultNamespace();
  }

  bool SelectorParser::IsAtEndIgnoringWhitespace(TokenRange tokens) const noexcept
  {
    tokens.DiscardWhitespace();
    return tokens.IsAtEnd();
  }

#pragma endregion
}