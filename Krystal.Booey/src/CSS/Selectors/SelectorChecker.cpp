#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDetailsElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHeadingElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderMenuList.hpp"
#include "Krystal.Booey/Style/Rules/RuleFeature.hpp"
#include "Krystal.Booey/Style/Rules/SelectorMatchingState.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include <algorithm>

namespace krys::boo::css
{
  /// @brief Where matching has got to: the simple selector being checked, the element it's checked
  /// against, and what the combinators crossed so far allow.
  struct SelectorChecker::LocalContext
  {
    const Selector *CurrentSelector;
    const boo::Element *Element;
    /// @brief The rightmost simple selector of the compound being checked.
    const Selector *FirstSelectorOfTheFragment;
    /// @brief The pseudo-element asked for, until a combinator is crossed.
    Maybe<style::PseudoElementIdentifier> RequestedPseudoElement;
    /// @brief Whether `Element` is the one being matched, not an ancestor or sibling reached through a
    /// combinator.
    bool IsMatchElement {true};
    /// @brief Whether `Element` is the one being matched or a sibling reached from it, rather than an
    /// ancestor; it decides whether positional style relations are recorded for the parent's children or
    /// its descendants.
    bool IsSubjectOrAdjacentElement {true};
    /// @brief Whether the selector is an argument of a functional pseudo-class.
    bool InFunctionalPseudoClass {false};
    /// @brief Whether a pseudo-element can match here: only in the rightmost compound, and not in arguments.
    bool PseudoElementEffective {true};
    /// @brief Whether the compound so far has `::selection`, which, unlike other pseudo-elements, may be
    /// followed by more of the compound.
    bool HasSelectionPseudo {false};
    /// @brief Whether `Element` is a shadow host seen from inside its shadow tree, where it's featureless
    /// and only `:host` (or `:scope`, `:is(:host)`) matches.
    bool MustMatchHostPseudoClass {false};
    bool MatchedHostPseudoClass {false};
    /// @brief Whether the selector is inside an odd number of `:not()`s.
    bool IsNegation {false};

    LocalContext(const Selector &selector, const boo::Element &element,
                 Maybe<style::PseudoElementIdentifier> requestedPseudoElement) noexcept
        : CurrentSelector(&selector), Element(&element), FirstSelectorOfTheFragment(&selector),
          RequestedPseudoElement(krys::move(requestedPseudoElement))
    {
    }
  };

  namespace
  {
    KRYS_NODISCARD bool IsHTMLElementInHTMLDocument(const Element &element) noexcept
    {
      return Is<HTMLElement>(element) && Is<HTMLDocument>(element.NodeDocument());
    }

    KRYS_NODISCARD bool IsFirstChildElement(const Element &element) noexcept
    {
      return !dom::TreeQueries::PreviousElementSibling(element);
    }

    KRYS_NODISCARD bool IsLastChildElement(const Element &element) noexcept
    {
      return !dom::TreeQueries::NextElementSibling(element);
    }

    /// @brief How many of @p element's previous (or, if @p forward, next) element siblings satisfy
    /// @p predicate.
    template <typename TPredicate>
    KRYS_NODISCARD int32 CountSiblings(const Element &element, bool forward, TPredicate predicate) noexcept
    {
      int32 count = 0;
      for (const auto *sibling = forward ? dom::TreeQueries::NextElementSibling(element)
                                         : dom::TreeQueries::PreviousElementSibling(element);
           sibling; sibling = forward ? dom::TreeQueries::NextElementSibling(*sibling)
                                      : dom::TreeQueries::PreviousElementSibling(*sibling))
      {
        if (predicate(*sibling))
        {
          ++count;
        }
      }
      return count;
    }

    KRYS_NODISCARD bool IsFirstOfType(const Element &element, const QualifiedName &type) noexcept
    {
      for (const auto *sibling = dom::TreeQueries::PreviousElementSibling(element); sibling;
           sibling = dom::TreeQueries::PreviousElementSibling(*sibling))
      {
        if (sibling->TagQualifiedName() == type)
        {
          return false;
        }
      }
      return true;
    }

    KRYS_NODISCARD bool IsLastOfType(const Element &element, const QualifiedName &type) noexcept
    {
      for (const auto *sibling = dom::TreeQueries::NextElementSibling(element); sibling;
           sibling = dom::TreeQueries::NextElementSibling(*sibling))
      {
        if (sibling->TagQualifiedName() == type)
        {
          return false;
        }
      }
      return true;
    }

    KRYS_NODISCARD int32 CountElementsBefore(const Element &element) noexcept
    {
      int32 count = 0;
      for (const auto *sibling = dom::TreeQueries::PreviousElementSibling(element); sibling;
           sibling = dom::TreeQueries::PreviousElementSibling(*sibling))
      {
        // A sibling whose index style resolution has recorded saves counting the rest.
        if (auto index = sibling->ChildIndex())
        {
          return count + static_cast<int32>(index);
        }
        ++count;
      }
      return count;
    }

    KRYS_NODISCARD int32 CountElementsOfTypeBefore(const Element &element, const QualifiedName &type) noexcept
    {
      return CountSiblings(element, false,
                           [&](const Element &sibling) { return sibling.TagQualifiedName() == type; });
    }

    KRYS_NODISCARD int32 CountElementsAfter(const Element &element) noexcept
    {
      return CountSiblings(element, true, [](const Element &) { return true; });
    }

    KRYS_NODISCARD int32 CountElementsOfTypeAfter(const Element &element, const QualifiedName &type) noexcept
    {
      return CountSiblings(element, true,
                           [&](const Element &sibling) { return sibling.TagQualifiedName() == type; });
    }

    /// @brief @p context moved to its element's parent for a child or descendant combinator: from a shadow
    /// tree's top-level element, the shadow host, which then must match `:host`; from the host itself,
    /// nothing, as the host has no ancestors inside its shadow tree.
    KRYS_NODISCARD SelectorChecker::LocalContext
      LocalContextForParent(const SelectorChecker::LocalContext &context) noexcept
    {
      SelectorChecker::LocalContext parentContext(context);
      parentContext.IsMatchElement = false;
      parentContext.IsSubjectOrAdjacentElement = false;
      if (parentContext.MustMatchHostPseudoClass)
      {
        parentContext.Element = nullptr;
      }
      else if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(context.Element->ParentNode()))
      {
        parentContext.Element = shadowRoot->Host();
        parentContext.MustMatchHostPseudoClass = true;
      }
      else
      {
        parentContext.Element = context.Element->ParentElement();
      }
      return parentContext;
    }

    /// @brief Whether @p element is a top-level element of a virtual scoping root (the document fragment or
    /// shadow root a query is scoped to), which Selectors makes its parent, and the rest of the complex
    /// selector, from @p selector leftwards, matches that root: a compound of only `:scope`, as nothing else
    /// matches it and it has no ancestors or siblings.
    /// @note Otherwise a shadow root's elements go on to match `:host` against the host
    /// (LocalContextForParent).
    KRYS_NODISCARD bool
      MatchesVirtualScopingRootAsParent(const SelectorChecker::CheckingContext &checkingContext,
                                        const Element &element, const Selector &selector) noexcept
    {
      const auto *scope = checkingContext.Scope.get();
      if (!scope || Is<Element>(*scope) || element.ParentNode() != scope)
      {
        return false;
      }
      for (const auto *simple = &selector; simple->IsScopePseudoClass();)
      {
        const auto *preceding = simple->PrecedingComplexSelectorComponent();
        if (!preceding)
        {
          return true;
        }
        if (simple->Relation() != SelectorRelation::Compounding)
        {
          return false;
        }
        simple = preceding;
      }
      return false;
    }

    KRYS_NODISCARD bool StartsWith(DOMStringView value, DOMStringView prefix, bool caseSensitive) noexcept
    {
      if (value.size() < prefix.size())
      {
        return false;
      }
      return caseSensitive ? value.starts_with(prefix)
                           : text::IsASCIICaselessEqual(value.substr(0, prefix.size()), prefix);
    }

    KRYS_NODISCARD bool EndsWith(DOMStringView value, DOMStringView suffix, bool caseSensitive) noexcept
    {
      if (value.size() < suffix.size())
      {
        return false;
      }
      return caseSensitive ? value.ends_with(suffix)
                           : text::IsASCIICaselessEqual(value.substr(value.size() - suffix.size()), suffix);
    }

    KRYS_NODISCARD size_t Find(DOMStringView value, DOMStringView search, size_t start,
                               bool caseSensitive) noexcept
    {
      if (caseSensitive)
      {
        return value.find(search, start);
      }
      for (size_t i = start; i + search.size() <= value.size(); ++i)
      {
        if (text::IsASCIICaselessEqual(value.substr(i, search.size()), search))
        {
          return i;
        }
      }
      return DOMStringView::npos;
    }

    /// @brief Whether @p value is one of the whitespace-separated words of @p list.
    KRYS_NODISCARD bool ListIncludes(DOMStringView list, DOMStringView value, bool caseSensitive) noexcept
    {
      for (size_t at = Find(list, value, 0, caseSensitive); at != DOMStringView::npos;
           at = Find(list, value, at + 1, caseSensitive))
      {
        const size_t end = at + value.size();
        if ((at == 0 || text::IsASCIIWhitespace(list[at - 1]))
            && (end == list.size() || text::IsASCIIWhitespace(list[end])))
        {
          return true;
        }
      }
      return false;
    }

    // https://drafts.csswg.org/selectors/#attribute-selectors
    KRYS_NODISCARD bool AttributeValueMatches(DOMStringView value, SelectorMatch match,
                                              DOMStringView selectorValue, bool caseSensitive) noexcept
    {
      switch (match)
      {
        case SelectorMatch::AttributeExists: return true;
        case SelectorMatch::AttributeEquals:
          return caseSensitive ? selectorValue == value : text::IsASCIICaselessEqual(selectorValue, value);
        case SelectorMatch::AttributeIncludes:
          // A value that is empty or has whitespace can't be one of the words.
          return !selectorValue.empty()
                 && std::ranges::none_of(selectorValue, [](char8 c) { return text::IsASCIIWhitespace(c); })
                 && ListIncludes(value, selectorValue, caseSensitive);
        // The substring forms never match an empty value.
        case SelectorMatch::AttributeSubstring:
          return !selectorValue.empty()
                 && Find(value, selectorValue, 0, caseSensitive) != DOMStringView::npos;
        case SelectorMatch::AttributePrefix:
          return !selectorValue.empty() && StartsWith(value, selectorValue, caseSensitive);
        case SelectorMatch::AttributeSuffix:
          return !selectorValue.empty() && EndsWith(value, selectorValue, caseSensitive);
        case SelectorMatch::AttributeDash:
          return StartsWith(value, selectorValue, caseSensitive)
                 && (value.size() == selectorValue.size() || value[selectorValue.size()] == u8'-');
        default: krys_debug_assert(false); return false;
      }
    }

    /// @brief Whether attribute @p name matches a selector's name: the same local name, and the same
    /// namespace unless the selector's prefix is `*`.
    KRYS_NODISCARD bool NameMatchesFilter(const QualifiedName &name, const CSSOMStringAtom &filterPrefix,
                                          const CSSOMStringAtom &filterLocalName,
                                          const CSSOMStringAtom &filterNamespaceURI) noexcept
    {
      return filterLocalName == name.LocalName()
             && (filterPrefix == dom::StarAtom() || filterNamespaceURI == name.NamespaceURI());
    }

    KRYS_NODISCARD bool AnyAttributeMatches(const Element &element, const Selector &selector,
                                            const QualifiedName &selectorAttr, bool caseSensitive) noexcept
    {
      // HTML elements' attribute names are lowercase in an HTML document, and selectors match them so.
      const auto &localName =
        IsHTMLElementInHTMLDocument(element) ? selectorAttr.LocalNameLower() : selectorAttr.LocalName();
      return std::ranges::any_of(element.AttributeList(),
                                 [&](const auto &attribute)
                                 {
                                   return NameMatchesFilter(attribute->GetQualifiedName(),
                                                            selectorAttr.NamespacePrefix(), localName,
                                                            selectorAttr.NamespaceURI())
                                          && AttributeValueMatches(attribute->Value(), selector.Match(),
                                                                   selector.Value().View(), caseSensitive);
                                 });
    }

    /// @brief Whether `:hover` or `:active` may match an element that isn't a link, which the
    /// [quirk](https://quirks.spec.whatwg.org/#the-active-and-hover-quirk) forbids for a compound of only
    /// `:hover`, `:active` and the universal selector, outside a functional pseudo-class's arguments.
    KRYS_NODISCARD bool
      CanMatchHoverOrActiveInQuirksMode(const SelectorChecker::LocalContext &context) noexcept
    {
      if (context.InFunctionalPseudoClass)
      {
        return true;
      }
      for (const auto *selector = context.FirstSelectorOfTheFragment; selector;
           selector = selector->PrecedingComplexSelectorComponent())
      {
        switch (selector->Match())
        {
          case SelectorMatch::Type:
            if (selector->TagQualifiedName() != dom::AnyQualifiedName())
            {
              return true;
            }
            break;
          case SelectorMatch::PseudoClass:
            if (selector->PseudoClass() != SelectorPseudoClass::Hover
                && selector->PseudoClass() != SelectorPseudoClass::Active)
            {
              return true;
            }
            break;
          case SelectorMatch::Id:
          case SelectorMatch::Class:
          case SelectorMatch::AttributeEquals:
          case SelectorMatch::AttributeExists:
          case SelectorMatch::AttributeIncludes:
          case SelectorMatch::AttributeDash:
          case SelectorMatch::AttributeSubstring:
          case SelectorMatch::AttributePrefix:
          case SelectorMatch::AttributeSuffix:
          case SelectorMatch::PseudoElement:                  return true;
          case SelectorMatch::HasScope:
          case SelectorMatch::NestingParent:
          case SelectorMatch::None:
          case SelectorMatch::ForgivingUnknown:
          case SelectorMatch::ForgivingUnknownNestContaining: krys_debug_assert(false); break;
        }
        // A pseudo-element after the compound makes it not only :hover and :active.
        if (selector->Relation() == SelectorRelation::ShadowDescendant
            || selector->Relation() == SelectorRelation::ShadowPartDescendant)
        {
          return true;
        }
        if (selector->Relation() != SelectorRelation::Compounding)
        {
          return false;
        }
      }
      return false;
    }

    // https://drafts.csswg.org/selectors/#type-selectors
    KRYS_NODISCARD bool TagMatches(const Element &element, const Selector &simpleSelector) noexcept
    {
      const auto &name = simpleSelector.TagQualifiedName();
      if (name == dom::AnyQualifiedName())
      {
        return true;
      }
      const auto &localName =
        IsHTMLElementInHTMLDocument(element) ? simpleSelector.TagLocalNameLower() : name.LocalName();
      if (localName != dom::StarAtom() && localName != element.LocalName())
      {
        return false;
      }
      return name.NamespaceURI() == dom::StarAtom() || name.NamespaceURI() == element.NamespaceURI();
    }

    /// @brief The tokens of the element's `part` attribute.
    KRYS_NODISCARD List<CSSOMStringAtom> PartNames(const Element &element) noexcept
    {
      List<CSSOMStringAtom> names;
      auto part = element.GetAttribute(u8"part");
      if (!part)
      {
        return names;
      }
      DOMStringView remaining = *part;
      while (!remaining.empty())
      {
        auto start = std::ranges::find_if_not(remaining, [](char8 c) { return text::IsASCIIWhitespace(c); });
        auto end =
          std::ranges::find_if(start, remaining.end(), [](char8 c) { return text::IsASCIIWhitespace(c); });
        if (start != end)
        {
          CSSOMStringAtom name(DOMStringView(start, end));
          if (std::ranges::find(names, name) == names.end())
          {
            names.push_back(name);
          }
        }
        remaining = DOMStringView(end, remaining.end());
      }
      return names;
    }

    /// @brief Whether an attribute selector compares values case-sensitively: not with the `i` flag, and,
    /// under @p htmlRules, not for an HTML attribute whose values HTML compares without case (unless the
    /// `s` flag says otherwise).
    KRYS_NODISCARD bool IsCaseSensitiveAttributeMatch(const Selector &selector, bool htmlRules) noexcept
    {
      if (selector.IsAttributeValueMatchingCaseInsensitive())
      {
        return false;
      }
      return !htmlRules || selector.IsAttributeValueMatchingCaseSensitive()
             || HTMLDocument::IsCaseSensitiveAttribute(selector.Attribute());
    }

    /// @brief Records that @p element's style depends on others' in the way @p type says, when resolving
    /// style. Runs of `AffectsNextSibling` on consecutive siblings are merged into one, counted.
    void AddStyleRelation(SelectorChecker::CheckingContext &checkingContext, const Element &element,
                          style::Relation::Type type, uint32 value = 1) noexcept
    {
      krys_debug_assert(value == 1 || type == style::Relation::NthChildIndex
                        || type == style::Relation::AffectedByEmpty);
      if (checkingContext.ResolvingMode != SelectorChecker::Mode::ResolvingStyle)
      {
        return;
      }
      if (type == style::Relation::AffectsNextSibling && !checkingContext.StyleRelations.empty())
      {
        auto &last = checkingContext.StyleRelations.back();
        if (last.RelationType == style::Relation::AffectsNextSibling
            && last.Element == dom::TreeQueries::NextElementSibling(element))
        {
          ++last.Value;
          last.Element = &element;
          return;
        }
      }
      checkingContext.StyleRelations.emplace_back(element, type, value);
    }
  }

  SelectorChecker::SelectorChecker(const Document &document) noexcept
      : _strictParsing(!document.InQuirksMode()), _documentIsHTML(Is<HTMLDocument>(document))
  {
  }

  bool SelectorChecker::Match(const Selector &selector, const Element &element,
                              CheckingContext &checkingContext) const noexcept
  {
    const auto &requestedPseudoElement = checkingContext.RequestedPseudoElement();
    LocalContext context(selector, element, requestedPseudoElement);
    if (checkingContext.StyleScopeOrdinal == style::ScopeOrdinal::Shadow)
    {
      // A rule from the element's own shadow tree sees it as a featureless host.
      krys_debug_assert(dom::TreeQueries::GetShadowRoot(element));
      context.MustMatchHostPseudoClass = true;
    }

    EnumSet<PseudoElementType> collectedPseudoElements;
    auto result = MatchRecursively(checkingContext, context, collectedPseudoElements);
    if (result.Outcome != MatchOutcome::SelectorMatches)
    {
      return false;
    }
    if (requestedPseudoElement)
    {
      return collectedPseudoElements.contains(requestedPseudoElement->Type);
    }
    if (!collectedPseudoElements)
    {
      return true;
    }
    // Matching the element for one of its pseudo-elements tells style resolution to resolve that too.
    if (checkingContext.ResolvingMode == Mode::ResolvingStyle)
    {
      checkingContext.PublicPseudoElements = collectedPseudoElements;
    }
    return checkingContext.ResolvingMode == Mode::StyleInvalidation || result.Type == MatchType::Element;
  }

  bool SelectorChecker::MatchHostPseudoClass(const Selector &selector, const Element &element,
                                             CheckingContext &checkingContext) const noexcept
  {
    if (!dom::TreeQueries::GetShadowRoot(element))
    {
      return false;
    }
    // :host(<compound>) also needs the host to match the compound.
    if (const auto *selectorList = selector.SubSelectors())
    {
      krys_debug_assert(selectorList->SelectorCount() == 1);
      LocalContext context(*selectorList->FirstSelectorComponent(), element, null);
      context.InFunctionalPseudoClass = true;
      context.PseudoElementEffective = false;
      EnumSet<PseudoElementType> ignoredPseudoElements;
      return MatchRecursively(checkingContext, context, ignoredPseudoElements).Outcome
             == MatchOutcome::SelectorMatches;
    }
    return true;
  }

  auto SelectorChecker::MatchRecursively(CheckingContext &checkingContext, LocalContext &context,
                                         EnumSet<PseudoElementType> &collectedPseudoElements) const noexcept
    -> MatchResult
  {
    auto matchType = MatchType::Element;
    if (!CheckOne(checkingContext, context, matchType))
    {
      return MatchResult::Fails(MatchOutcome::SelectorFailsLocally);
    }

    if (context.CurrentSelector->MatchesPseudoElement())
    {
      if (context.CurrentSelector->PseudoElement() == SelectorPseudoElement::UserAgentPart)
      {
        // :is() and the other logical combinations can't match pseudo-elements; one gets into their
        // arguments only through nesting resolution, which must then match nothing.
        if (context.InFunctionalPseudoClass)
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
        }
        // A user agent part is an element in a user agent shadow tree, with that part name.
        const auto *root = dom::TreeQueries::ContainingShadowRoot(*context.Element);
        if (!root || root->Mode() != dom::ShadowRootMode::UserAgent
            || context.Element->UserAgentPart() != context.CurrentSelector->Value())
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsLocally);
        }
      }
      else
      {
        // Other pseudo-elements match only in the rightmost compound, and never in a query.
        if (!context.PseudoElementEffective || checkingContext.ResolvingMode == Mode::QueryingRules)
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
        }
        if (auto type = Selector::StylePseudoElementTypeFor(context.CurrentSelector->PseudoElement()))
        {
          collectedPseudoElements.Add(*type);
        }
        matchType = MatchType::VirtualPseudoElementOnly;
      }
    }

    const auto relation = context.CurrentSelector->Relation();
    const auto *leftSelector = context.CurrentSelector->PrecedingComplexSelectorComponent();
    if (!leftSelector)
    {
      if (context.MustMatchHostPseudoClass && !context.MatchedHostPseudoClass)
      {
        return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
      }
      return MatchResult::Matches(matchType);
    }

    LocalContext nextContext(context);
    nextContext.CurrentSelector = leftSelector;
    if (relation != SelectorRelation::Compounding)
    {
      // Crossing a combinator: the requested pseudo-element must have been found in the rightmost compound.
      if (context.RequestedPseudoElement
          && !collectedPseudoElements.contains(context.RequestedPseudoElement->Type))
      {
        return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
      }
      nextContext.RequestedPseudoElement = null;
      // Pseudo-elements only match in the rightmost compound, except across a shadow descendant combinator.
      if (relation != SelectorRelation::ShadowDescendant)
      {
        nextContext.PseudoElementEffective = false;
      }
      nextContext.IsMatchElement = false;
      nextContext.FirstSelectorOfTheFragment = leftSelector;
    }

    // Matches the rest of the selector against nextContext's element. Pseudo-elements can't match there.
    const auto matchLeft = [&]
    {
      EnumSet<PseudoElementType> ignoredPseudoElements;
      auto result = MatchRecursively(checkingContext, nextContext, ignoredPseudoElements);
      krys_debug_assert(!nextContext.PseudoElementEffective && !ignoredPseudoElements);
      return result;
    };
    const auto isFinal = [](const MatchResult &result)
    {
      return result.Outcome == MatchOutcome::SelectorMatches
             || result.Outcome == MatchOutcome::SelectorFailsCompletely;
    };

    switch (relation)
    {
      case SelectorRelation::Descendant:
        // Try each ancestor in turn, until one matches or none can.
        while (true)
        {
          if (MatchesVirtualScopingRootAsParent(checkingContext, *nextContext.Element, *leftSelector))
          {
            return MatchResult::Matches(matchType);
          }
          nextContext = LocalContextForParent(nextContext);
          if (!nextContext.Element)
          {
            return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
          }
          if (auto result = matchLeft(); isFinal(result))
          {
            return MatchResult::UpdateWithMatchType(result, matchType);
          }
        }

      case SelectorRelation::Child:
      {
        if (MatchesVirtualScopingRootAsParent(checkingContext, *nextContext.Element, *leftSelector))
        {
          return MatchResult::Matches(matchType);
        }
        nextContext = LocalContextForParent(nextContext);
        if (!nextContext.Element)
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
        }
        // A sibling has the same parent, so it fails too.
        auto result = matchLeft();
        return isFinal(result) ? MatchResult::UpdateWithMatchType(result, matchType)
                               : MatchResult::Fails(MatchOutcome::SelectorFailsAllSiblings);
      }

      case SelectorRelation::NextSibling:
      {
        AddStyleRelation(checkingContext, *context.Element,
                         context.IsMatchElement ? style::Relation::AffectedByPreviousSibling
                                                : style::Relation::DescendantsAffectedByPreviousSibling);
        auto *previousElement = dom::TreeQueries::PreviousElementSibling(*context.Element);
        if (!previousElement)
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsAllSiblings);
        }
        AddStyleRelation(checkingContext, *previousElement, style::Relation::AffectsNextSibling);
        nextContext.Element = previousElement;
        return MatchResult::UpdateWithMatchType(matchLeft(), matchType);
      }

      case SelectorRelation::SubsequentSibling:
      {
        AddStyleRelation(checkingContext, *context.Element,
                         context.IsMatchElement ? style::Relation::AffectedByPreviousSibling
                                                : style::Relation::DescendantsAffectedByPreviousSibling);
        // Try each previous sibling in turn, until one matches or none can.
        for (nextContext.Element = dom::TreeQueries::PreviousElementSibling(*context.Element);
             nextContext.Element;
             nextContext.Element = dom::TreeQueries::PreviousElementSibling(*nextContext.Element))
        {
          AddStyleRelation(checkingContext, *nextContext.Element, style::Relation::AffectsNextSibling);
          if (auto result = matchLeft();
              isFinal(result) || result.Outcome == MatchOutcome::SelectorFailsAllSiblings)
          {
            return MatchResult::UpdateWithMatchType(result, matchType);
          }
        }
        return MatchResult::Fails(MatchOutcome::SelectorFailsAllSiblings);
      }

      case SelectorRelation::Compounding:
      {
        // Nothing may follow a pseudo-element in a compound, except `::selection`.
        nextContext.HasSelectionPseudo = collectedPseudoElements.contains(PseudoElementType::Selection);
        if ((context.IsMatchElement || checkingContext.ResolvingMode == Mode::CollectingRules)
            && collectedPseudoElements && !nextContext.HasSelectionPseudo)
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
        }
        return MatchResult::UpdateWithMatchType(
          MatchRecursively(checkingContext, nextContext, collectedPseudoElements), matchType);
      }

      // The shadow combinators continue matching at a host or slot in the tree the rule came from.
      case SelectorRelation::ShadowDescendant:
      case SelectorRelation::ShadowPartDescendant:
      case SelectorRelation::ShadowSlotted:
      {
        const Element *next = nullptr;
        if (relation == SelectorRelation::ShadowDescendant)
        {
          next = dom::TreeQueries::ShadowHost(*context.Element);
        }
        else if (relation == SelectorRelation::ShadowPartDescendant)
        {
          next = checkingContext.StyleScopeOrdinal == style::ScopeOrdinal::Element
                   ? dom::TreeQueries::ShadowHost(*context.Element)
                   : style::HostForScopeOrdinal(*context.Element, checkingContext.StyleScopeOrdinal);
          // ::part() rules from the element's own scope can only match if they apply to :host.
          nextContext.MustMatchHostPseudoClass =
            checkingContext.StyleScopeOrdinal == style::ScopeOrdinal::Element;
        }
        else
        {
          next = style::AssignedSlotForScopeOrdinal(*context.Element, checkingContext.StyleScopeOrdinal);
        }
        if (!next)
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
        }
        nextContext.Element = next;
        nextContext.IsSubjectOrAdjacentElement = false;
        EnumSet<PseudoElementType> ignoredPseudoElements;
        return MatchResult::UpdateWithMatchType(
          MatchRecursively(checkingContext, nextContext, ignoredPseudoElements), matchType);
      }
    }
    krys_debug_assert(false);
    return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
  }

  bool SelectorChecker::AttributeSelectorMatches(const Element &element, const QualifiedName &attributeName,
                                                 const CSSOMStringAtom &attributeValue,
                                                 const Selector &selector) noexcept
  {
    krys_debug_assert(selector.IsAttributeSelector());
    const auto &selectorAttribute = selector.Attribute();
    const bool isHTML = IsHTMLElementInHTMLDocument(element);
    const auto &selectorName = isHTML ? selectorAttribute.LocalNameLower() : selectorAttribute.LocalName();
    if (!NameMatchesFilter(attributeName, selectorAttribute.NamespacePrefix(), selectorName,
                           selectorAttribute.NamespaceURI()))
    {
      return false;
    }
    return AttributeValueMatches(attributeValue.View(), selector.Match(), selector.Value().View(),
                                 IsCaseSensitiveAttributeMatch(selector, isHTML));
  }

  namespace
  {
    /// @brief Records @p type on @p element's parent, if it's an element.
    void AddParentStyleRelation(SelectorChecker::CheckingContext &checkingContext, const Element &element,
                                style::Relation::Type type) noexcept
    {
      if (auto *parentElement = DynamicDowncast<Element>(element.ParentNode()))
      {
        AddStyleRelation(checkingContext, *parentElement, type);
      }
    }

    /// @brief Records on the element's parent that its children's (or, for an element reached through a
    /// descendant combinator, its descendants') styles depend on their position, counted from the start or
    /// (if @p backward) the end.
    void AddPositionalStyleRelation(SelectorChecker::CheckingContext &checkingContext,
                                    const SelectorChecker::LocalContext &context, bool backward) noexcept
    {
      if (backward)
      {
        AddParentStyleRelation(checkingContext, *context.Element,
                               context.IsSubjectOrAdjacentElement
                                 ? style::Relation::ChildrenAffectedByBackwardPositionalRules
                                 : style::Relation::DescendantsAffectedByBackwardPositionalRules);
      }
      else
      {
        AddParentStyleRelation(checkingContext, *context.Element,
                               context.IsSubjectOrAdjacentElement
                                 ? style::Relation::ChildrenAffectedByForwardPositionalRules
                                 : style::Relation::DescendantsAffectedByForwardPositionalRules);
      }
    }

    /// @brief Whether the parent is still being parsed, so that later siblings may yet arrive and anything
    /// counted from the end can't match.
    KRYS_NODISCARD bool ParentIsParsingChildren(const Element &element) noexcept
    {
      const auto *parentElement = DynamicDowncast<Element>(element.ParentNode());
      return parentElement && !parentElement->IsFinishedParsingChildren();
    }

    // https://drafts.csswg.org/selectors/#the-empty-pseudo
    KRYS_NODISCARD bool MatchesEmpty(SelectorChecker::CheckingContext &checkingContext,
                                     const Element &element) noexcept
    {
      bool empty = true;
      for (const auto *node = element.FirstChild(); node; node = node->NextSibling())
      {
        const auto *text = DynamicDowncast<Text>(*node);
        if (Is<Element>(*node) || (text && !text->Data().empty()))
        {
          empty = false;
          break;
        }
      }
      AddStyleRelation(checkingContext, element, style::Relation::AffectedByEmpty, empty);
      return empty;
    }

    // https://drafts.csswg.org/selectors/#the-first-child-pseudo
    KRYS_NODISCARD bool MatchesFirstChild(SelectorChecker::CheckingContext &checkingContext,
                                          const Element &element) noexcept
    {
      AddParentStyleRelation(checkingContext, element, style::Relation::ChildrenAffectedByFirstChildRules);
      if (!IsFirstChildElement(element))
      {
        return false;
      }
      AddStyleRelation(checkingContext, element, style::Relation::FirstChild);
      return true;
    }

    // https://drafts.csswg.org/selectors/#the-last-child-pseudo
    KRYS_NODISCARD bool MatchesLastChild(SelectorChecker::CheckingContext &checkingContext,
                                         const Element &element) noexcept
    {
      AddParentStyleRelation(checkingContext, element, style::Relation::ChildrenAffectedByLastChildRules);
      if (ParentIsParsingChildren(element) || !IsLastChildElement(element))
      {
        return false;
      }
      AddStyleRelation(checkingContext, element, style::Relation::LastChild);
      return true;
    }

    // https://drafts.csswg.org/selectors/#the-only-child-pseudo
    KRYS_NODISCARD bool MatchesOnlyChild(SelectorChecker::CheckingContext &checkingContext,
                                         const Element &element) noexcept
    {
      AddParentStyleRelation(checkingContext, element, style::Relation::ChildrenAffectedByFirstChildRules);
      AddParentStyleRelation(checkingContext, element, style::Relation::ChildrenAffectedByLastChildRules);
      const bool firstChild = IsFirstChildElement(element);
      const bool onlyChild = firstChild && IsLastChildElement(element) && !ParentIsParsingChildren(element);
      if (firstChild)
      {
        AddStyleRelation(checkingContext, element, style::Relation::FirstChild);
      }
      if (onlyChild)
      {
        AddStyleRelation(checkingContext, element, style::Relation::LastChild);
      }
      return onlyChild;
    }

    // https://drafts.csswg.org/selectors/#typed-child-index
    KRYS_NODISCARD bool MatchesFirstOfType(SelectorChecker::CheckingContext &checkingContext,
                                           const SelectorChecker::LocalContext &context) noexcept
    {
      AddPositionalStyleRelation(checkingContext, context, false);
      return IsFirstOfType(*context.Element, context.Element->TagQualifiedName());
    }

    KRYS_NODISCARD bool MatchesLastOfType(SelectorChecker::CheckingContext &checkingContext,
                                          const SelectorChecker::LocalContext &context) noexcept
    {
      AddPositionalStyleRelation(checkingContext, context, true);
      return !ParentIsParsingChildren(*context.Element)
             && IsLastOfType(*context.Element, context.Element->TagQualifiedName());
    }

    KRYS_NODISCARD bool MatchesOnlyOfType(SelectorChecker::CheckingContext &checkingContext,
                                          const SelectorChecker::LocalContext &context) noexcept
    {
      // FIXME: This walks all the element's siblings each time it's checked, which is slow.
      AddPositionalStyleRelation(checkingContext, context, false);
      AddPositionalStyleRelation(checkingContext, context, true);
      const auto &element = *context.Element;
      return !ParentIsParsingChildren(element) && IsFirstOfType(element, element.TagQualifiedName())
             && IsLastOfType(element, element.TagQualifiedName());
    }

    // https://drafts.csswg.org/selectors/#the-nth-of-type-pseudo
    KRYS_NODISCARD bool MatchesNthOfType(SelectorChecker::CheckingContext &checkingContext,
                                         const SelectorChecker::LocalContext &context,
                                         const Selector &selector) noexcept
    {
      AddPositionalStyleRelation(checkingContext, context, false);
      const auto &element = *context.Element;
      return selector.MatchNth(1 + CountElementsOfTypeBefore(element, element.TagQualifiedName()));
    }

    // https://drafts.csswg.org/selectors/#the-nth-last-of-type-pseudo
    KRYS_NODISCARD bool MatchesNthLastOfType(SelectorChecker::CheckingContext &checkingContext,
                                             const SelectorChecker::LocalContext &context,
                                             const Selector &selector) noexcept
    {
      AddPositionalStyleRelation(checkingContext, context, true);
      const auto &element = *context.Element;
      return !ParentIsParsingChildren(element)
             && selector.MatchNth(1 + CountElementsOfTypeAfter(element, element.TagQualifiedName()));
    }

    // https://drafts.csswg.org/selectors-5/#headings
    KRYS_NODISCARD bool MatchesHeading(const Element &element, const Selector &selector) noexcept
    {
      const auto *heading = DynamicDowncast<HTMLHeadingElement>(element);
      if (!heading)
      {
        return false;
      }
      const auto *levels = selector.IntegerList();
      return !levels || levels->empty()
             || std::ranges::contains(*levels, static_cast<int32>(heading->ComputedHeadingLevel()));
    }

    // https://html.spec.whatwg.org/#selector-open
    KRYS_NODISCARD bool MatchesOpen(const Element &element) noexcept
    {
      if (Is<HTMLDetailsElement>(element) || Is<HTMLDialogElement>(element))
      {
        return element.HasAttribute(u8"open");
      }
      if (const auto *select = DynamicDowncast<HTMLSelectElement>(element))
      {
        const auto *menuList = DynamicDowncast<RenderMenuList>(select->Renderer());
        return (menuList && menuList->PopupIsVisible()) || select->IsPickerOpen();
      }
      if (const auto *input = DynamicDowncast<HTMLInputElement>(element))
      {
        return input->IsPresentingAttachedView();
      }
      return false;
    }

    /// @brief @p element's part names as the tree a rule comes from sees them: mapped through the
    /// `exportparts` of each shadow root between the element and that tree.
    KRYS_NODISCARD List<CSSOMStringAtom>
      PartNamesInRuleScope(const SelectorChecker::CheckingContext &checkingContext,
                           const Element &element) noexcept
    {
      auto names = PartNames(element);
      if (checkingContext.StyleScopeOrdinal == style::ScopeOrdinal::Element)
      {
        return names;
      }
      const auto *ruleScopeHost = style::HostForScopeOrdinal(element, checkingContext.StyleScopeOrdinal);
      List<CSSOMStringAtom> translated;
      for (const auto &name : names)
      {
        List<CSSOMStringAtom> mapped {name};
        for (const auto *shadowRoot = dom::TreeQueries::ContainingShadowRoot(element);
             shadowRoot && shadowRoot->Host() != ruleScopeHost && !mapped.empty();
             shadowRoot = dom::TreeQueries::ContainingShadowRoot(*shadowRoot->Host()))
        {
          List<CSSOMStringAtom> next;
          const auto &partMappings = shadowRoot->GetPartMappings();
          for (const auto &mappedName : mapped)
          {
            if (auto it = partMappings.find(mappedName); it != partMappings.end())
            {
              next.insert(next.end(), it->second.begin(), it->second.end());
            }
          }
          mapped = krys::move(next);
        }
        translated.insert(translated.end(), mapped.begin(), mapped.end());
      }
      return translated;
    }
  }

  bool SelectorChecker::CheckOne(CheckingContext &checkingContext, LocalContext &context,
                                 MatchType &matchType) const noexcept
  {
    const auto &element = *context.Element;
    const auto &selector = *context.CurrentSelector;

    if (context.MustMatchHostPseudoClass)
    {
      // A featureless host matches only :host, :scope (when it's the scoping root), a logical combination
      // containing those, and pseudo-elements.
      // FIXME: :host combined with :not(), which depends on how logical combinations match a featureless
      // element (https://drafts.csswg.org/selectors/#featureless).
      const bool isNotPseudoClass =
        selector.Match() == SelectorMatch::PseudoClass && selector.PseudoClass() == SelectorPseudoClass::Not;
      if (!selector.IsHostPseudoClass() && !selector.MatchesPseudoElement() && !selector.IsScopePseudoClass()
          && (!selector.SubSelectors() || isNotPseudoClass))
      {
        return false;
      }
    }

    switch (selector.Match())
    {
      case SelectorMatch::Type:  return TagMatches(element, selector);
      case SelectorMatch::Class: return element.HasClassName(selector.Value().View());
      case SelectorMatch::Id:
        krys_debug_assert(selector.Value() != null);
        return element.IdForStyleResolution() == selector.Value().View();
      case SelectorMatch::AttributeExists:
      case SelectorMatch::AttributeEquals:
      case SelectorMatch::AttributeDash:
      case SelectorMatch::AttributeIncludes:
      case SelectorMatch::AttributePrefix:
      case SelectorMatch::AttributeSuffix:
      case SelectorMatch::AttributeSubstring:
        return element.HasAttributes()
               && AnyAttributeMatches(
                 element, selector, selector.Attribute(),
                 IsCaseSensitiveAttributeMatch(selector, _documentIsHTML && Is<HTMLElement>(element)));
      // Unparseable selectors match nothing, nor does an `&` that had no parent rule to resolve to.
      case SelectorMatch::ForgivingUnknown:
      case SelectorMatch::ForgivingUnknownNestContaining:
      case SelectorMatch::NestingParent:                  return false;
      case SelectorMatch::HasScope:
        checkingContext.MatchedInsideScope = true;
        return &element == checkingContext.HasScope || checkingContext.MatchesAllHasScopes;
      case SelectorMatch::PseudoClass:   return CheckPseudoClass(checkingContext, context, matchType);
      case SelectorMatch::PseudoElement: return CheckPseudoElement(checkingContext, context);
      case SelectorMatch::None:          return true;
    }
    return true;
  }

  bool SelectorChecker::CheckPseudoClass(CheckingContext &checkingContext, LocalContext &context,
                                         MatchType &matchType) const noexcept
  {
    const auto &element = *context.Element;
    const auto &selector = *context.CurrentSelector;

    switch (selector.PseudoClass())
    {
#pragma region Logical combinations

      // https://drafts.csswg.org/selectors/#negation
      case SelectorPseudoClass::Not:
      {
        const auto *selectorList = selector.SubSelectors();
        for (const auto &subselector : *selectorList)
        {
          LocalContext subcontext(context);
          subcontext.IsNegation = !context.IsNegation;
          subcontext.InFunctionalPseudoClass = true;
          subcontext.PseudoElementEffective = false;
          subcontext.CurrentSelector = &subselector;
          subcontext.FirstSelectorOfTheFragment = selectorList->FirstSelectorComponent();
          EnumSet<PseudoElementType> ignoredPseudoElements;
          if (MatchRecursively(checkingContext, subcontext, ignoredPseudoElements).Outcome
              == MatchOutcome::SelectorMatches)
          {
            krys_debug_assert(!ignoredPseudoElements);
            return false;
          }
        }
        return true;
      }

      // https://drafts.csswg.org/selectors/#matches and #zero-matches
      case SelectorPseudoClass::Is:
      case SelectorPseudoClass::Where:
      {
        krys_debug_assert(selector.SubSelectors());
        bool matched = false;
        auto localMatchType = MatchType::VirtualPseudoElementOnly;
        for (const auto &subselector : *selector.SubSelectors())
        {
          LocalContext subcontext(context);
          subcontext.InFunctionalPseudoClass = true;
          subcontext.CurrentSelector = &subselector;
          subcontext.FirstSelectorOfTheFragment = &subselector;
          subcontext.RequestedPseudoElement = null;
          EnumSet<PseudoElementType> subCollectedPseudoElements;
          auto result = MatchRecursively(checkingContext, subcontext, subCollectedPseudoElements);
          context.MatchedHostPseudoClass |= subcontext.MatchedHostPseudoClass;
          // An argument matching through a pseudo-element doesn't count: these can't match pseudo-elements.
          if (subCollectedPseudoElements || result.Outcome != MatchOutcome::SelectorMatches)
          {
            continue;
          }
          matched = true;
          if (result.Type == MatchType::Element)
          {
            localMatchType = MatchType::Element;
          }
        }
        if (matched)
        {
          matchType = localMatchType;
        }
        return matched;
      }

      // https://drafts.csswg.org/selectors/#relational
      case SelectorPseudoClass::Has:
        return std::ranges::any_of(*selector.SubSelectors(), [&](const Selector &hasSelector)
                                   { return MatchHasPseudoClass(checkingContext, element, hasSelector); });

#pragma endregion

#pragma region Tree-structural

      case SelectorPseudoClass::Root:
        return &element == dom::TreeQueries::DocumentElement(element.NodeDocument());
      case SelectorPseudoClass::Empty:       return MatchesEmpty(checkingContext, element);
      case SelectorPseudoClass::FirstChild:  return MatchesFirstChild(checkingContext, element);
      case SelectorPseudoClass::LastChild:   return MatchesLastChild(checkingContext, element);
      case SelectorPseudoClass::OnlyChild:   return MatchesOnlyChild(checkingContext, element);
      case SelectorPseudoClass::FirstOfType: return MatchesFirstOfType(checkingContext, context);
      case SelectorPseudoClass::LastOfType:  return MatchesLastOfType(checkingContext, context);
      case SelectorPseudoClass::OnlyOfType:  return MatchesOnlyOfType(checkingContext, context);
      case SelectorPseudoClass::NthOfType:   return MatchesNthOfType(checkingContext, context, selector);
      case SelectorPseudoClass::NthLastOfType:
        return MatchesNthLastOfType(checkingContext, context, selector);

      // https://drafts.csswg.org/selectors/#the-nth-child-pseudo
      case SelectorPseudoClass::NthChild:
      {
        AddPositionalStyleRelation(checkingContext, context, false);
        // `:nth-child(An+B of S)` counts only the siblings matching S, and the element must match it too.
        const auto *selectorList = selector.SubSelectors();
        if (selectorList && !MatchSelectorList(checkingContext, context, element, *selectorList))
        {
          return false;
        }
        int32 position = 1;
        if (selectorList)
        {
          for (const auto *sibling = dom::TreeQueries::PreviousElementSibling(element); sibling;
               sibling = dom::TreeQueries::PreviousElementSibling(*sibling))
          {
            position += MatchSelectorList(checkingContext, context, *sibling, *selectorList);
          }
        }
        else
        {
          position += CountElementsBefore(element);
          AddStyleRelation(checkingContext, element, style::Relation::NthChildIndex,
                           static_cast<uint32>(position));
        }
        return selector.MatchNth(position);
      }

      // https://drafts.csswg.org/selectors/#the-nth-last-child-pseudo
      case SelectorPseudoClass::NthLastChild:
      {
        const auto *selectorList = selector.SubSelectors();
        if (const auto *parentElement = DynamicDowncast<Element>(element.ParentNode()))
        {
          if (selectorList)
          {
            if (!MatchSelectorList(checkingContext, context, element, *selectorList))
            {
              return false;
            }
            AddStyleRelation(checkingContext, *parentElement,
                             style::Relation::ChildrenAffectedByBackwardPositionalRules);
          }
          else
          {
            AddPositionalStyleRelation(checkingContext, context, true);
          }
          if (!parentElement->IsFinishedParsingChildren())
          {
            return false;
          }
        }
        int32 position = 1;
        if (selectorList)
        {
          for (const auto *sibling = dom::TreeQueries::NextElementSibling(element); sibling;
               sibling = dom::TreeQueries::NextElementSibling(*sibling))
          {
            position += MatchSelectorList(checkingContext, context, *sibling, *selectorList);
          }
        }
        else
        {
          position += CountElementsAfter(element);
        }
        return selector.MatchNth(position);
      }

#pragma endregion

#pragma region Location and user action

      // There's no history, so every link is unvisited.
      case SelectorPseudoClass::AnyLink:
      case SelectorPseudoClass::Link:         return IsLink(element);
      case SelectorPseudoClass::Visited:      return false;
      case SelectorPseudoClass::Target:       return &element == element.NodeDocument().CSSTarget();
      case SelectorPseudoClass::Focus:        return MatchesFocusPseudoClass(element);
      case SelectorPseudoClass::FocusVisible: return MatchesFocusVisiblePseudoClass(element);
      case SelectorPseudoClass::FocusWithin:  return MatchesFocusWithinPseudoClass(element);
      case SelectorPseudoClass::Hover:
        return (_strictParsing || IsLink(element) || CanMatchHoverOrActiveInQuirksMode(context))
               && element.Hovered();
      case SelectorPseudoClass::Active:
        return (_strictParsing || IsLink(element) || CanMatchHoverOrActiveInQuirksMode(context))
               && element.Active();

#pragma endregion

#pragma region Input

      // There's no autofill (see OVERVIEW.md), so it never matches.
      case SelectorPseudoClass::Autofill:         return false;
      case SelectorPseudoClass::Enabled:          return MatchesEnabledPseudoClass(element);
      case SelectorPseudoClass::Disabled:         return MatchesDisabledPseudoClass(element);
      case SelectorPseudoClass::ReadOnly:         return MatchesReadOnlyPseudoClass(element);
      case SelectorPseudoClass::ReadWrite:        return MatchesReadWritePseudoClass(element);
      case SelectorPseudoClass::PlaceholderShown: return IsPlaceholderShown(element);
      case SelectorPseudoClass::Default:          return MatchesDefaultPseudoClass(element);
      case SelectorPseudoClass::Checked:          return IsChecked(element);
      case SelectorPseudoClass::Indeterminate:    return MatchesIndeterminatePseudoClass(element);
      case SelectorPseudoClass::Valid:            return element.MatchesValidPseudoClass();
      case SelectorPseudoClass::Invalid:          return element.MatchesInvalidPseudoClass();
      case SelectorPseudoClass::InRange:          return element.IsInRange();
      case SelectorPseudoClass::OutOfRange:       return element.IsOutOfRange();
      case SelectorPseudoClass::Required:         return IsRequiredFormControl(element);
      case SelectorPseudoClass::Optional:         return IsOptionalFormControl(element);
      case SelectorPseudoClass::UserValid:        return element.MatchesUserValidPseudoClass();
      case SelectorPseudoClass::UserInvalid:      return element.MatchesUserInvalidPseudoClass();

#pragma endregion

#pragma region Linguistic

      case SelectorPseudoClass::Dir: return MatchesDirPseudoClass(element, selector.Argument());
      case SelectorPseudoClass::Lang:
        krys_debug_assert(selector.LangList() && !selector.LangList()->empty());
        return MatchesLangPseudoClass(element, *selector.LangList());

#pragma endregion

#pragma region Element state

      case SelectorPseudoClass::Defined:     return IsDefinedElement(element);
      case SelectorPseudoClass::State:       return element.HasCustomState(selector.Argument());
      case SelectorPseudoClass::PopoverOpen: return element.IsPopoverShowing();
      case SelectorPseudoClass::Open:        return MatchesOpen(element);
      case SelectorPseudoClass::Heading:     return MatchesHeading(element, selector);
      case SelectorPseudoClass::Modal:
      {
        // FIXME: Fullscreen elements, once there are any.
        const auto *dialog = DynamicDowncast<HTMLDialogElement>(element);
        return dialog && dialog->IsModal();
      }
      // NOLINTBEGIN(bugprone-branch-clone): each group has its own FIXME.
      case SelectorPseudoClass::Fullscreen:
      case SelectorPseudoClass::PictureInPicture:
        // FIXME: Fullscreen and picture-in-picture.
        return false;
      case SelectorPseudoClass::Future:
      case SelectorPseudoClass::Past:
        // FIXME: WebVTT cues.
        return false;
      case SelectorPseudoClass::Playing:
      case SelectorPseudoClass::Paused:
      case SelectorPseudoClass::Seeking:
      case SelectorPseudoClass::Buffering:
      case SelectorPseudoClass::Stalled:
      case SelectorPseudoClass::Muted:
      case SelectorPseudoClass::VolumeLocked:
        // FIXME: Media element playback state.
        return false;
        // NOLINTEND(bugprone-branch-clone)

#pragma endregion

#pragma region Scoping

      // https://drafts.csswg.org/selectors/#the-scope-pseudo
      case SelectorPseudoClass::Scope:
      {
        // Invalidation doesn't track @scope rules, so :scope is assumed to match (or, negated, not to).
        if (checkingContext.ResolvingMode == Mode::StyleInvalidation)
        {
          return !context.IsNegation;
        }
        // A virtual scoping root (a document fragment or shadow root) matches no element; the child and
        // descendant combinators match `:scope` against it (MatchesVirtualScopingRootAsParent).
        const Node *scopingRoot = checkingContext.Scope
                                    ? static_cast<const Node *>(checkingContext.Scope.get())
                                    : dom::TreeQueries::DocumentElement(element.NodeDocument());
        if (&element != scopingRoot)
        {
          return false;
        }
        // A shadow host that is the scoping root is matched as if by :host.
        if (dom::TreeQueries::GetShadowRoot(element))
        {
          context.MatchedHostPseudoClass = true;
        }
        return true;
      }

      // https://drafts.csswg.org/css-scoping-1/#host-selector
      case SelectorPseudoClass::Host:
        if (!context.MustMatchHostPseudoClass || !MatchHostPseudoClass(selector, element, checkingContext))
        {
          return false;
        }
        context.MatchedHostPseudoClass = true;
        return true;

#pragma endregion

#pragma region Internal

      case SelectorPseudoClass::InternalSelectBaseContents:
      {
        const auto *select = DynamicDowncast<HTMLSelectElement>(element);
        return select && select->ContentsUseBaseAppearance();
      }
      case SelectorPseudoClass::InternalHtmlDocument: return Is<HTMLDocument>(element.NodeDocument());

#pragma endregion
    }
    return false;
  }

  bool SelectorChecker::CheckPseudoElement(CheckingContext &checkingContext,
                                           LocalContext &context) const noexcept
  {
    const auto &element = *context.Element;
    const auto &selector = *context.CurrentSelector;

    // Matches @p subselector against the element, as a functional pseudo-element's argument.
    const auto argumentMatches = [&](const Selector &subselector)
    {
      LocalContext subcontext(context);
      subcontext.CurrentSelector = &subselector;
      subcontext.FirstSelectorOfTheFragment = &subselector;
      subcontext.InFunctionalPseudoClass = true;
      subcontext.PseudoElementEffective = false;
      EnumSet<PseudoElementType> ignoredPseudoElements;
      return MatchRecursively(checkingContext, subcontext, ignoredPseudoElements).Outcome
             == MatchOutcome::SelectorMatches;
    };

    switch (selector.PseudoElement())
    {
      case SelectorPseudoElement::Cue: return std::ranges::any_of(*selector.SubSelectors(), argumentMatches);

      // https://drafts.csswg.org/css-scoping-1/#slotted-pseudo
      case SelectorPseudoElement::Slotted:
        // It matches the flattened tree, where a slot in a shadow tree has been replaced by what's in it.
        if (!dom::SlotAlgorithms::GetAssignedSlot(element)
            || (Is<HTMLSlotElement>(element) && dom::TreeQueries::ContainingShadowRoot(element)))
        {
          return false;
        }
        return argumentMatches(*selector.SubSelectors()->FirstSelectorComponent());

      // https://drafts.csswg.org/css-shadow-parts-1/#part
      case SelectorPseudoElement::Part:
      {
        krys_debug_assert(selector.ArgumentList());
        const auto partNames = PartNamesInRuleScope(checkingContext, element);
        return std::ranges::all_of(*selector.ArgumentList(), [&](const CSSOMStringAtom &part)
                                   { return std::ranges::contains(partNames, part); });
      }

      case SelectorPseudoElement::Highlight:
      {
        // Without a request, it matches so the element is marked as having the pseudo-element.
        const auto &requestedPseudoElement = checkingContext.RequestedPseudoElement();
        if (!requestedPseudoElement)
        {
          return true;
        }
        return requestedPseudoElement->Type == PseudoElementType::Highlight && selector.ArgumentList()
               && (*selector.ArgumentList())[0] == requestedPseudoElement->NameArgument;
      }

      default: return true;
    }
  }

  bool SelectorChecker::MatchSelectorList(CheckingContext &checkingContext, const LocalContext &context,
                                          const Element &element,
                                          const SelectorList &selectorList) const noexcept
  {
    // Every argument is checked, not just up to the first that matches, so each records its style relations.
    bool matched = false;
    for (const auto &subselector : selectorList)
    {
      LocalContext subcontext(context);
      subcontext.Element = &element;
      subcontext.CurrentSelector = &subselector;
      subcontext.FirstSelectorOfTheFragment = &subselector;
      subcontext.InFunctionalPseudoClass = true;
      subcontext.PseudoElementEffective = false;
      EnumSet<PseudoElementType> ignoredPseudoElements;
      if (MatchRecursively(checkingContext, subcontext, ignoredPseudoElements).Outcome
          == MatchOutcome::SelectorMatches)
      {
        krys_debug_assert(!ignoredPseudoElements);
        matched = true;
      }
    }
    return matched;
  }

  // https://drafts.csswg.org/selectors/#relational
  bool SelectorChecker::MatchHasPseudoClass(CheckingContext &checkingContext, const Element &element,
                                            const Selector &hasSelector) const noexcept
  {
    // The parser rejects :has() inside :has(), but nesting resolution can still put one there.
    if (checkingContext.DisallowHasPseudoClass)
    {
      return false;
    }

    // Which elements the relative selector can reach from the anchor: children, descendants, following
    // siblings, or their descendants.
    const auto matchElement = style::ComputeHasPseudoClassMatchElement(hasSelector);
    switch (matchElement)
    {
      case style::MatchElement::HasChild:
      case style::MatchElement::HasDescendant:
        if (!dom::TreeQueries::FirstElementChild(element))
        {
          return false;
        }
        break;
      case style::MatchElement::HasSibling:
      case style::MatchElement::HasSiblingDescendant:
        if (!dom::TreeQueries::NextElementSibling(element))
        {
          return false;
        }
        break;
      default: break;
    }

    // Results are cached per element and selector across the matches sharing a SelectorMatchingState. A
    // failure that never reached the anchor (FailsSubtree) also rules out the elements under it.
    auto *cache = checkingContext.SelectorMatchingState
                    ? &checkingContext.SelectorMatchingState->HasPseudoClassMatchCache
                    : nullptr;
    const auto cachedMatch = [&](const Element &cachedElement)
    {
      auto it = cache->find(style::MakeHasPseudoClassCacheKey(cachedElement, hasSelector));
      return it == cache->end() ? style::HasPseudoClassMatch::None : it->second;
    };
    if (cache)
    {
      switch (cachedMatch(element))
      {
        case style::HasPseudoClassMatch::Matches:      return true;
        case style::HasPseudoClassMatch::Fails:
        case style::HasPseudoClassMatch::FailsSubtree: return false;
        case style::HasPseudoClassMatch::None:         break;
      }
    }

    // A Bloom filter of the identifiers around the element may reject the selector outright. It's built
    // only when the element is checked a second time with another selector; the first check only notes it.
    if (checkingContext.SelectorMatchingState)
    {
      if (auto type = style::HasSelectorFilter::TypeForMatchElement(matchElement))
      {
        auto &filters = checkingContext.SelectorMatchingState->HasPseudoClassSelectorFilters;
        auto [it, isNewEntry] = filters.try_emplace(style::MakeHasPseudoClassFilterKey(element, *type));
        if (!isNewEntry)
        {
          if (!it->second)
          {
            it->second = CreateUnique<style::HasSelectorFilter>(element, *type);
          }
          if (it->second->Reject(hasSelector))
          {
            return false;
          }
        }
      }
    }

    // The relative selector is matched with the element as its anchor, recording relations separately.
    CheckingContext hasCheckingContext(Mode::ResolvingStyle);
    hasCheckingContext.Scope = checkingContext.Scope;
    hasCheckingContext.HasScope = &element;
    hasCheckingContext.DisallowHasPseudoClass = true;

    bool matchedInsideScope = false;
    const auto checkRelative = [&](const Element &elementToCheck)
    {
      LocalContext hasContext(hasSelector, elementToCheck, null);
      hasContext.InFunctionalPseudoClass = true;
      hasContext.PseudoElementEffective = false;
      EnumSet<PseudoElementType> ignoredPseudoElements;
      const bool matched = MatchRecursively(hasCheckingContext, hasContext, ignoredPseudoElements).Outcome
                           == MatchOutcome::SelectorMatches;
      matchedInsideScope |= hasCheckingContext.MatchedInsideScope;
      return matched;
    };
    const auto checkDescendants = [&](const Element &root)
    {
      for (const auto *node = dom::TreeQueries::Next(root, &root); node;)
      {
        const auto *descendant = DynamicDowncast<Element>(*node);
        if (!descendant)
        {
          node = dom::TreeQueries::Next(*node, &root);
          continue;
        }
        if (checkRelative(*descendant))
        {
          return true;
        }
        // Skip a subtree the cache says the selector can't match in.
        if (cache && dom::TreeQueries::FirstElementChild(*descendant)
            && cachedMatch(*descendant) == style::HasPseudoClassMatch::FailsSubtree)
        {
          node = dom::TreeQueries::NextSkippingChildren(*descendant, &root);
          continue;
        }
        node = dom::TreeQueries::Next(*descendant, &root);
      }
      return false;
    };
    const auto anyFollowingSibling = [&](auto check)
    {
      for (const auto *sibling = dom::TreeQueries::NextElementSibling(element); sibling;
           sibling = dom::TreeQueries::NextElementSibling(*sibling))
      {
        if (check(*sibling))
        {
          return true;
        }
      }
      return false;
    };

    const bool result = [&]
    {
      switch (matchElement)
      {
        // :has(> a)
        case style::MatchElement::HasChild:
          for (const auto *child = dom::TreeQueries::FirstElementChild(element); child;
               child = dom::TreeQueries::NextElementSibling(*child))
          {
            if (checkRelative(*child))
            {
              return true;
            }
          }
          return false;
        // :has(a)
        case style::MatchElement::HasDescendant:
          // An ancestor whose subtree failed rules this element's out too.
          if (cache)
          {
            for (const auto *ancestor = element.ParentElement(); ancestor;
                 ancestor = ancestor->ParentElement())
            {
              if (cachedMatch(*ancestor) == style::HasPseudoClassMatch::FailsSubtree)
              {
                return false;
              }
            }
          }
          return checkDescendants(element);
        // :has(+ a) and :has(~ a).
        // FIXME: `+` only needs the next sibling, but every following one is checked.
        case style::MatchElement::HasSibling:           return anyFollowingSibling(checkRelative);
        // :has(+ a b) and :has(~ a b).
        // FIXME: `+` only needs the next sibling's descendants, but every following sibling's are checked.
        case style::MatchElement::HasSiblingDescendant: return anyFollowingSibling(checkDescendants);
        default:                                        krys_debug_assert(false); return false;
      }
    }();

    if (cache)
    {
      auto cacheResult = result               ? style::HasPseudoClassMatch::Matches
                         : matchedInsideScope ? style::HasPseudoClassMatch::Fails
                                              : style::HasPseudoClassMatch::FailsSubtree;
      cache->try_emplace(style::MakeHasPseudoClassCacheKey(element, hasSelector), cacheResult);
    }

    // Pass the relative selector's style relations on: the first- and last-child ones as they are, the
    // positional ones as a :has() with a positional pseudo-class, and the rest not at all.
    for (const auto &relation : hasCheckingContext.StyleRelations)
    {
      switch (relation.RelationType)
      {
        case style::Relation::ChildrenAffectedByForwardPositionalRules:
        case style::Relation::ChildrenAffectedByBackwardPositionalRules:
          checkingContext.StyleRelations.emplace_back(
            *relation.Element, style::Relation::AffectedByHasWithPositionalPseudoClass);
          break;
        case style::Relation::ChildrenAffectedByFirstChildRules:
        case style::Relation::ChildrenAffectedByLastChildRules:
          checkingContext.StyleRelations.push_back(relation);
          break;
        case style::Relation::AffectedByEmpty:
        case style::Relation::AffectedByPreviousSibling:
        case style::Relation::DescendantsAffectedByPreviousSibling:
        case style::Relation::AffectsNextSibling:
        case style::Relation::DescendantsAffectedByForwardPositionalRules:
        case style::Relation::DescendantsAffectedByBackwardPositionalRules:
        case style::Relation::FirstChild:
        case style::Relation::LastChild:
        case style::Relation::NthChildIndex:                                break;
        case style::Relation::AffectedByHasWithPositionalPseudoClass:       krys_debug_assert(false); break;
      }
    }
    return result;
  }
}
