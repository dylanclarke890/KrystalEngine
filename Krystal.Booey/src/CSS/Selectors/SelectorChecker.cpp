#include "Krystal.Booey/CSS/Selectors/SelectorChecker.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorList.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Style/RuleFeature.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/SelectorMatchingState.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include <algorithm>

namespace krys::boo::css
{
  enum class VisitedMatchType : uint8
  {
    Disabled,
    Enabled
  };

  struct SelectorChecker::LocalContext
  {
    const Selector *CurrentSelector;
    const dom::Element *Element;
    VisitedMatchType VisitedType;
    const Selector *FirstSelectorOfTheFragment;
    Maybe<style::PseudoElementIdentifier> RequestedPseudoElement;
    bool IsMatchElement {true};
    bool IsSubjectOrAdjacentElement {true};
    bool InFunctionalPseudoClass {false};
    bool PseudoElementEffective {true};
    bool HasSelectionPseudo {false};
    bool HasViewTransitionPseudo {false};
    bool MustMatchHostPseudoClass {false};
    bool MatchedHostPseudoClass {false};
    bool IsNegation {false};

    LocalContext(const Selector &selector, const dom::Element &element, VisitedMatchType visitedMatchType,
                 Maybe<style::PseudoElementIdentifier> requestedPseudoElement) noexcept
        : CurrentSelector(&selector), Element(&element), VisitedType(visitedMatchType),
          FirstSelectorOfTheFragment(&selector), RequestedPseudoElement(krys::move(requestedPseudoElement))
    {
    }
  };

  namespace
  {
    KRYS_NODISCARD bool IsHTMLElementInHTMLDocument(const dom::Element &element) noexcept
    {
      return Is<html::HTMLElement>(element) && Is<dom::HTMLDocument>(element.NodeDocument());
    }

    KRYS_NODISCARD bool IsFirstChildElement(const dom::Element &element) noexcept
    {
      return !dom::TreeQueries::PreviousElementSibling(element);
    }

    KRYS_NODISCARD bool IsLastChildElement(const dom::Element &element) noexcept
    {
      return !dom::TreeQueries::NextElementSibling(element);
    }

    KRYS_NODISCARD bool IsFirstOfType(const dom::Element &element, const dom::QualifiedName &type) noexcept
    {
      for (auto *sibling = dom::TreeQueries::PreviousElementSibling(element); sibling;
           sibling = dom::TreeQueries::PreviousElementSibling(*sibling))
      {
        if (sibling->TagQualifiedName() == type)
        {
          return false;
        }
      }
      return true;
    }

    KRYS_NODISCARD bool IsLastOfType(const dom::Element &element, const dom::QualifiedName &type) noexcept
    {
      for (auto *sibling = dom::TreeQueries::NextElementSibling(element); sibling;
           sibling = dom::TreeQueries::NextElementSibling(*sibling))
      {
        if (sibling->TagQualifiedName() == type)
        {
          return false;
        }
      }
      return true;
    }

    KRYS_NODISCARD int32 CountElementsBefore(const dom::Element &element) noexcept
    {
      int32 count = 0;
      for (auto *sibling = dom::TreeQueries::PreviousElementSibling(element); sibling;
           sibling = dom::TreeQueries::PreviousElementSibling(*sibling))
      {
        auto index = sibling->ChildIndex();
        if (index)
        {
          count += static_cast<int32>(index);
          break;
        }
        ++count;
      }
      return count;
    }

    KRYS_NODISCARD int32 CountElementsOfTypeBefore(const dom::Element &element,
                                                   const dom::QualifiedName &type) noexcept
    {
      int32 count = 0;
      for (auto *sibling = dom::TreeQueries::PreviousElementSibling(element); sibling;
           sibling = dom::TreeQueries::PreviousElementSibling(*sibling))
      {
        if (sibling->TagQualifiedName() == type)
        {
          ++count;
        }
      }
      return count;
    }

    KRYS_NODISCARD int32 CountElementsAfter(const dom::Element &element) noexcept
    {
      int32 count = 0;
      for (auto *sibling = dom::TreeQueries::NextElementSibling(element); sibling;
           sibling = dom::TreeQueries::NextElementSibling(*sibling))
      {
        ++count;
      }
      return count;
    }

    KRYS_NODISCARD int32 CountElementsOfTypeAfter(const dom::Element &element,
                                                  const dom::QualifiedName &type) noexcept
    {
      int32 count = 0;
      for (auto *sibling = dom::TreeQueries::NextElementSibling(element); sibling;
           sibling = dom::TreeQueries::NextElementSibling(*sibling))
      {
        if (sibling->TagQualifiedName() == type)
        {
          ++count;
        }
      }
      return count;
    }

    KRYS_NODISCARD bool
      HasViewTransitionPseudoElement(EnumSet<PseudoElementType> collectedPseudoElements) noexcept
    {
      auto functionalPseudoElementSet = EnumSet {
        PseudoElementType::ViewTransition,          PseudoElementType::ViewTransitionGroup,
        PseudoElementType::ViewTransitionImagePair, PseudoElementType::ViewTransitionNew,
        PseudoElementType::ViewTransitionOld,
      };
      return collectedPseudoElements.ContainsAny(functionalPseudoElementSet);
    }

    KRYS_NODISCARD SelectorChecker::LocalContext
      LocalContextForParent(const SelectorChecker::LocalContext &context) noexcept
    {
      SelectorChecker::LocalContext updatedContext(context);
      // Disable :visited matching when we see the first link.
      if (IsLink(*context.Element))
      {
        updatedContext.VisitedType = VisitedMatchType::Disabled;
      }

      updatedContext.IsMatchElement = false;
      updatedContext.IsSubjectOrAdjacentElement = false;

      if (updatedContext.MustMatchHostPseudoClass)
      {
        updatedContext.Element = nullptr;
        return updatedContext;
      }

      // Move to the shadow host if the parent is the shadow root and mark that we must match :host
      if (auto *shadowRoot = DynamicDowncast<dom::ShadowRoot>(context.Element->ParentNode()))
      {
        updatedContext.Element = shadowRoot->Host();
        updatedContext.MustMatchHostPseudoClass = true;
        return updatedContext;
      }

      updatedContext.Element = context.Element->ParentElement();
      return updatedContext;
    }

    KRYS_NODISCARD bool StartsWith(dom::DOMStringView value, dom::DOMStringView prefix,
                                   bool caseSensitive) noexcept
    {
      if (value.size() < prefix.size())
      {
        return false;
      }
      return caseSensitive ? value.starts_with(prefix)
                           : text::IsASCIICaselessEqual(value.substr(0, prefix.size()), prefix);
    }

    KRYS_NODISCARD bool EndsWith(dom::DOMStringView value, dom::DOMStringView suffix,
                                 bool caseSensitive) noexcept
    {
      if (value.size() < suffix.size())
      {
        return false;
      }
      return caseSensitive ? value.ends_with(suffix)
                           : text::IsASCIICaselessEqual(value.substr(value.size() - suffix.size()), suffix);
    }

    KRYS_NODISCARD size_t Find(dom::DOMStringView value, dom::DOMStringView search, size_t start,
                               bool caseSensitive) noexcept
    {
      if (caseSensitive)
      {
        return value.find(search, start);
      }
      if (search.size() > value.size())
      {
        return dom::DOMStringView::npos;
      }
      for (size_t i = start; i + search.size() <= value.size(); ++i)
      {
        if (text::IsASCIICaselessEqual(value.substr(i, search.size()), search))
        {
          return i;
        }
      }
      return dom::DOMStringView::npos;
    }

    KRYS_NODISCARD bool AttributeValueMatches(dom::DOMStringView value, SelectorMatch match,
                                              dom::DOMStringView selectorValue, bool caseSensitive) noexcept
    {
      switch (match)
      {
        case SelectorMatch::AttributeExists: break;
        case SelectorMatch::AttributeEquals:
          if (caseSensitive ? selectorValue != value : !text::IsASCIICaselessEqual(selectorValue, value))
          {
            return false;
          }
          break;
        case SelectorMatch::AttributeIncludes:
        {
          // Ignore empty selectors or selectors containing spaces.
          if (selectorValue.empty()
              || std::ranges::any_of(selectorValue, [](char8 c) { return text::IsASCIIWhitespace(c); }))
          {
            return false;
          }

          size_t startSearchAt = 0;
          while (true)
          {
            size_t foundPos = Find(value, selectorValue, startSearchAt, caseSensitive);
            if (foundPos == dom::DOMStringView::npos)
            {
              return false;
            }
            if (!foundPos || text::IsASCIIWhitespace(value[foundPos - 1]))
            {
              size_t endStr = foundPos + selectorValue.size();
              if (endStr == value.size() || text::IsASCIIWhitespace(value[endStr]))
              {
                break; // We found a match.
              }
            }

            // No match. Keep looking.
            startSearchAt = foundPos + 1;
          }
          break;
        }
        case SelectorMatch::AttributeSubstring:
        {
          bool valueContainsSelectorValue =
            Find(value, selectorValue, 0, caseSensitive) != dom::DOMStringView::npos;

          if (!valueContainsSelectorValue || selectorValue.empty())
          {
            return false;
          }

          break;
        }
        case SelectorMatch::AttributePrefix:
          if (selectorValue.empty())
          {
            return false;
          }
          if (!StartsWith(value, selectorValue, caseSensitive))
          {
            return false;
          }
          break;
        case SelectorMatch::AttributeSuffix:
          if (selectorValue.empty())
          {
            return false;
          }
          if (!EndsWith(value, selectorValue, caseSensitive))
          {
            return false;
          }
          break;
        case SelectorMatch::AttributeDash:
          if (value.size() < selectorValue.size())
          {
            return false;
          }
          if (!StartsWith(value, selectorValue, caseSensitive))
          {
            return false;
          }
          // It they start the same, check for exact match or following '-':
          if (value.size() != selectorValue.size() && value[selectorValue.size()] != u8'-')
          {
            return false;
          }
          break;
        default: krys_debug_assert(false); return false;
      }

      return true;
    }

    KRYS_NODISCARD bool NameMatchesFilter(const dom::QualifiedName &name, const CSSOMStringAtom &filterPrefix,
                                          const CSSOMStringAtom &filterLocalName,
                                          const CSSOMStringAtom &filterNamespaceURI) noexcept
    {
      if (filterLocalName != name.LocalName())
      {
        return false;
      }
      return filterPrefix == dom::StarAtom() || filterNamespaceURI == name.NamespaceURI();
    }

    KRYS_NODISCARD bool AnyAttributeMatches(const dom::Element &element, const Selector &selector,
                                            const dom::QualifiedName &selectorAttr,
                                            bool caseSensitive) noexcept
    {
      bool isHTML = IsHTMLElementInHTMLDocument(element);
      auto &localName = isHTML ? selectorAttr.LocalNameLower() : selectorAttr.LocalName();
      for (auto &attribute : element.AttributeList())
      {
        if (!NameMatchesFilter(attribute->GetQualifiedName(), selectorAttr.NamespacePrefix(), localName,
                               selectorAttr.NamespaceURI()))
        {
          continue;
        }

        if (AttributeValueMatches(attribute->Value(), selector.Match(), selector.Value().View(),
                                  caseSensitive))
        {
          return true;
        }
      }

      return false;
    }

    KRYS_NODISCARD bool
      CanMatchHoverOrActiveInQuirksMode(const SelectorChecker::LocalContext &context) noexcept
    {
      // For quirks mode, follow this: http://quirks.spec.whatwg.org/#the-:active-and-:hover-quirk
      // In quirks mode, a compound selector 'selector' that matches the following conditions must not match
      // elements that would not also match the ':any-link' selector.
      //
      //    selector uses the ':active' or ':hover' pseudo-classes.
      //    selector does not use a type selector.
      //    selector does not use an attribute selector.
      //    selector does not use an ID selector.
      //    selector does not use a class selector.
      //    selector does not use a pseudo-class selector other than ':active' and ':hover'.
      //    selector does not use a pseudo-element selector.
      //    selector is not part of an argument to a functional pseudo-class or pseudo-element.
      if (context.InFunctionalPseudoClass)
      {
        return true;
      }

      for (auto *selector = context.FirstSelectorOfTheFragment; selector;
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
          {
            auto pseudoClass = selector->PseudoClass();
            if (pseudoClass != SelectorPseudoClass::Hover && pseudoClass != SelectorPseudoClass::Active)
            {
              return true;
            }
            break;
          }
          case SelectorMatch::Id:
          case SelectorMatch::Class:
          case SelectorMatch::AttributeEquals:
          case SelectorMatch::AttributeExists:
          case SelectorMatch::AttributeIncludes:
          case SelectorMatch::AttributeDash:
          case SelectorMatch::AttributeSubstring:
          case SelectorMatch::AttributePrefix:
          case SelectorMatch::AttributeSuffix:
          case SelectorMatch::PagePseudoClass:
          case SelectorMatch::PseudoElement:                  return true;
          case SelectorMatch::HasScope:
          case SelectorMatch::NestingParent:
          case SelectorMatch::None:
          case SelectorMatch::ForgivingUnknown:
          case SelectorMatch::ForgivingUnknownNestContaining: krys_debug_assert(false); break;
        }

        auto relation = selector->Relation();
        if (relation == SelectorRelation::ShadowDescendant
            || relation == SelectorRelation::ShadowPartDescendant)
        {
          return true;
        }

        if (relation != SelectorRelation::Compounding)
        {
          return false;
        }
      }
      return false;
    }

    KRYS_NODISCARD bool TagMatches(const dom::Element &element, const Selector &simpleSelector) noexcept
    {
      auto &tagQName = simpleSelector.TagQualifiedName();

      if (tagQName == dom::AnyQualifiedName())
      {
        return true;
      }

      auto &localName =
        IsHTMLElementInHTMLDocument(element) ? simpleSelector.TagLocalNameLower() : tagQName.LocalName();

      if (localName != dom::StarAtom() && localName != element.LocalName())
      {
        return false;
      }
      auto &namespaceURI = tagQName.NamespaceURI();
      return namespaceURI == dom::StarAtom() || namespaceURI == element.NamespaceURI();
    }

    /// @brief The tokens of the element's `part` attribute.
    KRYS_NODISCARD List<CSSOMStringAtom> PartNames(const dom::Element &element) noexcept
    {
      List<CSSOMStringAtom> names;
      auto part = element.GetAttribute(u8"part");
      if (!part)
      {
        return names;
      }
      dom::DOMStringView remaining = *part;
      while (!remaining.empty())
      {
        auto start = std::ranges::find_if_not(remaining, [](char8 c) { return text::IsASCIIWhitespace(c); });
        auto end =
          std::ranges::find_if(start, remaining.end(), [](char8 c) { return text::IsASCIIWhitespace(c); });
        if (start != end)
        {
          CSSOMStringAtom name(dom::DOMStringView(start, end));
          if (std::ranges::find(names, name) == names.end())
          {
            names.push_back(name);
          }
        }
        remaining = dom::DOMStringView(end, remaining.end());
      }
      return names;
    }
  }

  static inline void AddStyleRelation(SelectorChecker::CheckingContext &checkingContext,
                                      const dom::Element &element, style::Relation::Type type,
                                      uint32 value = 1) noexcept
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

  SelectorChecker::SelectorChecker(const dom::Document &document) noexcept
      : _strictParsing(!document.InQuirksMode()), _documentIsHTML(Is<dom::HTMLDocument>(document))
  {
  }

  bool SelectorChecker::Match(const Selector &selector, const dom::Element &element,
                              CheckingContext &checkingContext) const noexcept
  {
    auto &requestedPseudoElement = checkingContext.RequestedPseudoElement();
    LocalContext context(selector, element,
                         checkingContext.ResolvingMode == Mode::QueryingRules ? VisitedMatchType::Disabled
                                                                              : VisitedMatchType::Enabled,
                         requestedPseudoElement);

    if (checkingContext.StyleScopeOrdinal == style::ScopeOrdinal::Shadow)
    {
      krys_debug_assert(dom::TreeQueries::GetShadowRoot(element));
      // Rules coming from the element's shadow tree must match :host pseudo class.
      context.MustMatchHostPseudoClass = true;
    }

    EnumSet<PseudoElementType> collectedPseudoElements;
    MatchResult result = MatchRecursively(checkingContext, context, collectedPseudoElements);
    if (result.Outcome != MatchOutcome::SelectorMatches)
    {
      return false;
    }

    if (requestedPseudoElement)
    {
      return collectedPseudoElements.Contains(requestedPseudoElement->Type);
    }

    if (collectedPseudoElements)
    {
      if (checkingContext.ResolvingMode == Mode::ResolvingStyle)
      {
        checkingContext.PublicPseudoElements = collectedPseudoElements;
      }

      return checkingContext.ResolvingMode == Mode::StyleInvalidation || result.Type == MatchType::Element;
    }
    return true;
  }

  bool SelectorChecker::MatchHostPseudoClass(const Selector &selector, const dom::Element &element,
                                             CheckingContext &checkingContext) const noexcept
  {
    if (!dom::TreeQueries::GetShadowRoot(element))
    {
      return false;
    }

    if (auto *selectorList = selector.SubSelectors())
    {
      krys_debug_assert(selectorList->SelectorCount() == 1);
      LocalContext context(*selectorList->FirstSelectorComponent(), element, VisitedMatchType::Enabled, null);
      context.InFunctionalPseudoClass = true;
      context.PseudoElementEffective = false;
      EnumSet<PseudoElementType> ignoredPseudoElements;
      if (MatchRecursively(checkingContext, context, ignoredPseudoElements).Outcome
          != MatchOutcome::SelectorMatches)
      {
        return false;
      }
    }
    return true;
  }

  // Recursive check of selectors and combinators
  // It can return 4 different values:
  // * SelectorMatches          - the selector matches the element e
  // * SelectorFailsLocally     - the selector fails for the element e
  // * SelectorFailsAllSiblings - the selector fails for e and any sibling of e
  // * SelectorFailsCompletely  - the selector fails for e and any sibling or ancestor of e
  auto SelectorChecker::MatchRecursively(CheckingContext &checkingContext, LocalContext &context,
                                         EnumSet<PseudoElementType> &collectedPseudoElements) const noexcept
    -> MatchResult
  {
    MatchType matchType = MatchType::Element;

    // The first selector has to match.
    if (!CheckOne(checkingContext, context, matchType))
    {
      return MatchResult::Fails(MatchOutcome::SelectorFailsLocally);
    }

    if (context.CurrentSelector->Match() == SelectorMatch::PseudoElement)
    {
      switch (context.CurrentSelector->PseudoElement())
      {
        case SelectorPseudoElement::UserAgentPart:
        {
          // In functional pseudo class, custom pseudo elements are always disabled.
          // FIXME: We should accept custom pseudo elements inside :is()/:matches().
          if (context.InFunctionalPseudoClass)
          {
            return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
          }
          if (auto *root = dom::TreeQueries::ContainingShadowRoot(*context.Element))
          {
            if (root->Mode() != dom::ShadowRootMode::UserAgent)
            {
              return MatchResult::Fails(MatchOutcome::SelectorFailsLocally);
            }

            if (context.Element->UserAgentPart() != context.CurrentSelector->Value())
            {
              return MatchResult::Fails(MatchOutcome::SelectorFailsLocally);
            }
          }
          else
          {
            return MatchResult::Fails(MatchOutcome::SelectorFailsLocally);
          }
          break;
        }
        default:
        {
          if (!context.PseudoElementEffective)
          {
            return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
          }

          if (checkingContext.ResolvingMode == Mode::QueryingRules)
          {
            return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
          }

          if (auto type = Selector::StylePseudoElementTypeFor(context.CurrentSelector->PseudoElement()))
          {
            collectedPseudoElements.Add(*type);
          }
          matchType = MatchType::VirtualPseudoElementOnly;
          break;
        }
      }
    }

    // The rest of the selectors has to match
    auto relation = context.CurrentSelector->Relation();

    // Prepare next selector
    const Selector *leftSelector = context.CurrentSelector->PrecedingComplexSelectorComponent();
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
      if (context.RequestedPseudoElement
          && !collectedPseudoElements.Contains(context.RequestedPseudoElement->Type))
      {
        return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
      }

      // Disable :visited matching when we try to match anything else than an ancestors.
      if (!context.CurrentSelector->HasDescendantOrChildRelation())
      {
        nextContext.VisitedType = VisitedMatchType::Disabled;
      }

      nextContext.RequestedPseudoElement = null;

      bool allowMultiplePseudoElements = relation == SelectorRelation::ShadowDescendant;
      // Virtual pseudo element is only effective in the rightmost fragment.
      if (!allowMultiplePseudoElements)
      {
        nextContext.PseudoElementEffective = false;
      }

      nextContext.IsMatchElement = false;
    }

    switch (relation)
    {
      case SelectorRelation::Descendant:
        nextContext = LocalContextForParent(nextContext);
        nextContext.FirstSelectorOfTheFragment = nextContext.CurrentSelector;
        for (; nextContext.Element; nextContext = LocalContextForParent(nextContext))
        {
          EnumSet<PseudoElementType> ignoredPseudoElements;
          MatchResult result = MatchRecursively(checkingContext, nextContext, ignoredPseudoElements);
          krys_debug_assert(!nextContext.PseudoElementEffective && !ignoredPseudoElements);

          if (result.Outcome == MatchOutcome::SelectorMatches
              || result.Outcome == MatchOutcome::SelectorFailsCompletely)
          {
            return MatchResult::UpdateWithMatchType(result, matchType);
          }
        }
        return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);

      case SelectorRelation::Child:
      {
        nextContext = LocalContextForParent(nextContext);
        if (!nextContext.Element)
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
        }
        nextContext.FirstSelectorOfTheFragment = nextContext.CurrentSelector;
        EnumSet<PseudoElementType> ignoredPseudoElements;
        MatchResult result = MatchRecursively(checkingContext, nextContext, ignoredPseudoElements);
        krys_debug_assert(!nextContext.PseudoElementEffective && !ignoredPseudoElements);

        if (result.Outcome == MatchOutcome::SelectorMatches
            || result.Outcome == MatchOutcome::SelectorFailsCompletely)
        {
          return MatchResult::UpdateWithMatchType(result, matchType);
        }
        return MatchResult::Fails(MatchOutcome::SelectorFailsAllSiblings);
      }

      case SelectorRelation::NextSibling:
      {
        auto siblingRelation = context.IsMatchElement ? style::Relation::AffectedByPreviousSibling
                                                      : style::Relation::DescendantsAffectedByPreviousSibling;
        AddStyleRelation(checkingContext, *context.Element, siblingRelation);

        auto *previousElement = dom::TreeQueries::PreviousElementSibling(*context.Element);
        if (!previousElement)
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsAllSiblings);
        }

        AddStyleRelation(checkingContext, *previousElement, style::Relation::AffectsNextSibling);

        nextContext.Element = previousElement;
        nextContext.FirstSelectorOfTheFragment = nextContext.CurrentSelector;
        EnumSet<PseudoElementType> ignoredPseudoElements;
        MatchResult result = MatchRecursively(checkingContext, nextContext, ignoredPseudoElements);
        krys_debug_assert(!nextContext.PseudoElementEffective && !ignoredPseudoElements);

        return MatchResult::UpdateWithMatchType(result, matchType);
      }
      case SelectorRelation::SubsequentSibling:
      {
        auto siblingRelation = context.IsMatchElement ? style::Relation::AffectedByPreviousSibling
                                                      : style::Relation::DescendantsAffectedByPreviousSibling;
        AddStyleRelation(checkingContext, *context.Element, siblingRelation);

        nextContext.Element = dom::TreeQueries::PreviousElementSibling(*context.Element);
        nextContext.FirstSelectorOfTheFragment = nextContext.CurrentSelector;
        for (; nextContext.Element;
             nextContext.Element = dom::TreeQueries::PreviousElementSibling(*nextContext.Element))
        {
          AddStyleRelation(checkingContext, *nextContext.Element, style::Relation::AffectsNextSibling);

          EnumSet<PseudoElementType> ignoredPseudoElements;
          MatchResult result = MatchRecursively(checkingContext, nextContext, ignoredPseudoElements);
          krys_debug_assert(!nextContext.PseudoElementEffective && !ignoredPseudoElements);

          if (result.Outcome == MatchOutcome::SelectorMatches
              || result.Outcome == MatchOutcome::SelectorFailsAllSiblings
              || result.Outcome == MatchOutcome::SelectorFailsCompletely)
          {
            return MatchResult::UpdateWithMatchType(result, matchType);
          }
        }
        return MatchResult::Fails(MatchOutcome::SelectorFailsAllSiblings);
      }
      case SelectorRelation::Compounding:
      {
        // a selector is invalid if something follows a pseudo-element
        // We make an exception for view transition pseudo elements and allow a set of pseudo classes (but
        // nothing else) to follow the pseudo elements.
        nextContext.HasSelectionPseudo = collectedPseudoElements.Contains(PseudoElementType::Selection);
        nextContext.HasViewTransitionPseudo = HasViewTransitionPseudoElement(collectedPseudoElements);

        if ((context.IsMatchElement || checkingContext.ResolvingMode == Mode::CollectingRules)
            && collectedPseudoElements && !nextContext.HasSelectionPseudo
            && !(nextContext.HasViewTransitionPseudo
                 && nextContext.CurrentSelector->Match() == SelectorMatch::PseudoClass))
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
        }

        MatchResult result = MatchRecursively(checkingContext, nextContext, collectedPseudoElements);

        return MatchResult::UpdateWithMatchType(result, matchType);
      }
      case SelectorRelation::ShadowDescendant:
      {
        auto *host = dom::TreeQueries::ShadowHost(*context.Element);
        if (!host)
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
        }

        nextContext.Element = host;
        nextContext.FirstSelectorOfTheFragment = nextContext.CurrentSelector;
        nextContext.IsSubjectOrAdjacentElement = false;
        EnumSet<PseudoElementType> ignoredPseudoElements;
        MatchResult result = MatchRecursively(checkingContext, nextContext, ignoredPseudoElements);

        return MatchResult::UpdateWithMatchType(result, matchType);
      }
      case SelectorRelation::ShadowPartDescendant:
      {
        // Continue matching in the scope where this rule came from.
        auto *host = checkingContext.StyleScopeOrdinal == style::ScopeOrdinal::Element
                       ? dom::TreeQueries::ShadowHost(*context.Element)
                       : style::HostForScopeOrdinal(*context.Element, checkingContext.StyleScopeOrdinal);
        if (!host)
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
        }

        nextContext.Element = host;
        nextContext.FirstSelectorOfTheFragment = nextContext.CurrentSelector;
        nextContext.IsSubjectOrAdjacentElement = false;
        // ::part rules from the element's own scope can only match if they apply to :host.
        nextContext.MustMatchHostPseudoClass =
          checkingContext.StyleScopeOrdinal == style::ScopeOrdinal::Element;
        EnumSet<PseudoElementType> ignoredPseudoElements;
        MatchResult result = MatchRecursively(checkingContext, nextContext, ignoredPseudoElements);

        return MatchResult::UpdateWithMatchType(result, matchType);
      }
      case SelectorRelation::ShadowSlotted:
      {
        // We continue matching in the scope where this rule came from.
        auto *slot = style::AssignedSlotForScopeOrdinal(*context.Element, checkingContext.StyleScopeOrdinal);
        if (!slot)
        {
          return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
        }

        nextContext.Element = slot;
        nextContext.FirstSelectorOfTheFragment = nextContext.CurrentSelector;
        nextContext.IsSubjectOrAdjacentElement = false;
        EnumSet<PseudoElementType> ignoredPseudoElements;
        MatchResult result = MatchRecursively(checkingContext, nextContext, ignoredPseudoElements);

        return MatchResult::UpdateWithMatchType(result, matchType);
      }
    }

    krys_debug_assert(false);
    return MatchResult::Fails(MatchOutcome::SelectorFailsCompletely);
  }

  bool SelectorChecker::AttributeSelectorMatches(const dom::Element &element,
                                                 const dom::QualifiedName &attributeName,
                                                 const CSSOMStringAtom &attributeValue,
                                                 const Selector &selector) noexcept
  {
    krys_debug_assert(selector.IsAttributeSelector());
    auto &selectorAttribute = selector.Attribute();
    bool isHTML = IsHTMLElementInHTMLDocument(element);
    auto &selectorName = isHTML ? selectorAttribute.LocalNameLower() : selectorAttribute.LocalName();
    if (!NameMatchesFilter(attributeName, selectorAttribute.NamespacePrefix(), selectorName,
                           selectorAttribute.NamespaceURI()))
    {
      return false;
    }
    bool caseSensitive = true;
    if (selector.IsAttributeValueMatchingCaseInsensitive())
    {
      caseSensitive = false;
    }
    else if (isHTML && !dom::HTMLDocument::IsCaseSensitiveAttribute(selector.Attribute()))
    {
      caseSensitive = false;
    }
    return AttributeValueMatches(attributeValue.View(), selector.Match(), selector.Value().View(),
                                 caseSensitive);
  }

  bool SelectorChecker::CheckOne(CheckingContext &checkingContext, LocalContext &context,
                                 MatchType &matchType) const noexcept
  {
    const dom::Element &element = *context.Element;
    const Selector &selector = *context.CurrentSelector;

    if (context.MustMatchHostPseudoClass)
    {
      // :host doesn't combine with anything except pseudo elements.
      bool isPseudoElement = selector.Match() == SelectorMatch::PseudoElement;
      // FIXME: We do not support combining :host with :not() functional pseudoclass. Combination with
      // functional pseudoclass has been allowed for the useful :is(:host) ; but combining with :not() doesn't
      // sound useful like :host():not(:not(:host)) https://bugs.webkit.org/show_bug.cgi?id=283062
      bool isNotPseudoClass =
        selector.Match() == SelectorMatch::PseudoClass && selector.PseudoClass() == SelectorPseudoClass::Not;

      // We can early return when we know it's neither :host, :scope (which can match when the scoping root is
      // the shadow host), a compound :is(:host) , a pseudo-element.
      if (!selector.IsHostPseudoClass() && !isPseudoElement && !selector.IsScopePseudoClass()
          && (!selector.SubSelectors() || isNotPseudoClass))
      {
        return false;
      }
    }

    if (selector.Match() == SelectorMatch::Type)
    {
      return TagMatches(element, selector);
    }

    if (selector.Match() == SelectorMatch::Class)
    {
      return element.HasClassName(selector.Value().View());
    }

    if (selector.Match() == SelectorMatch::Id)
    {
      krys_debug_assert(selector.Value() != CSSOMStringAtom::Null());
      return element.IdForStyleResolution() == selector.Value().View();
    }

    if (selector.IsAttributeSelector())
    {
      if (!element.HasAttributes())
      {
        return false;
      }

      auto &attr = selector.Attribute();
      bool caseSensitive = true;
      if (selector.IsAttributeValueMatchingCaseInsensitive())
      {
        caseSensitive = false;
      }
      else if (_documentIsHTML && Is<html::HTMLElement>(element)
               && !dom::HTMLDocument::IsCaseSensitiveAttribute(attr))
      {
        caseSensitive = false;
      }

      return AnyAttributeMatches(element, selector, attr, caseSensitive);
    }

    if (selector.Match() == SelectorMatch::ForgivingUnknown
        || selector.Match() == SelectorMatch::ForgivingUnknownNestContaining)
    {
      return false;
    }

    if (selector.Match() == SelectorMatch::NestingParent)
    {
      return false;
    }

    if (selector.Match() == SelectorMatch::HasScope)
    {
      checkingContext.MatchedInsideScope = true;
      return &element == checkingContext.HasScope || checkingContext.MatchesAllHasScopes;
    }

    if (selector.Match() == SelectorMatch::PseudoClass)
    {
      // Handle :not up front.
      if (selector.PseudoClass() == SelectorPseudoClass::Not)
      {
        const SelectorList *selectorList = selector.SubSelectors();

        for (auto &subselector : *selectorList)
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

      if (context.HasViewTransitionPseudo)
      {
        return CheckViewTransitionPseudoClass(checkingContext, element, selector);
      }

      // Normal element pseudo class checking.
      switch (selector.PseudoClass())
      {
          // Pseudo classes:
        case SelectorPseudoClass::Not: krys_debug_assert(false); break; // Already handled up above.
        case SelectorPseudoClass::Empty:
        {
          bool result = true;
          for (auto *node = element.FirstChild(); node; node = node->NextSibling())
          {
            if (Is<dom::Element>(*node))
            {
              result = false;
              break;
            }
            if (auto *textNode = DynamicDowncast<dom::Text>(*node))
            {
              if (!textNode->Data().empty())
              {
                result = false;
                break;
              }
            }
          }
          AddStyleRelation(checkingContext, *context.Element, style::Relation::AffectedByEmpty, result);

          return result;
        }
        case SelectorPseudoClass::FirstChild:
        {
          // first-child matches the first child that is an element
          bool isFirstChild = IsFirstChildElement(element);
          if (auto *parentElement = DynamicDowncast<dom::Element>(element.ParentNode()))
          {
            AddStyleRelation(checkingContext, *parentElement,
                             style::Relation::ChildrenAffectedByFirstChildRules);
          }
          if (!isFirstChild)
          {
            break;
          }
          AddStyleRelation(checkingContext, element, style::Relation::FirstChild);
          return true;
        }
        case SelectorPseudoClass::FirstOfType:
        {
          // first-of-type matches the first element of its type
          if (auto *parentElement = DynamicDowncast<dom::Element>(element.ParentNode()))
          {
            auto relation = context.IsSubjectOrAdjacentElement
                              ? style::Relation::ChildrenAffectedByForwardPositionalRules
                              : style::Relation::DescendantsAffectedByForwardPositionalRules;
            AddStyleRelation(checkingContext, *parentElement, relation);
          }
          return IsFirstOfType(element, element.TagQualifiedName());
        }
        case SelectorPseudoClass::LastChild:
        {
          // last-child matches the last child that is an element
          bool isLastChild = IsLastChildElement(element);
          if (auto *parentElement = DynamicDowncast<dom::Element>(element.ParentNode()))
          {
            if (!parentElement->IsFinishedParsingChildren())
            {
              isLastChild = false;
            }
            AddStyleRelation(checkingContext, *parentElement,
                             style::Relation::ChildrenAffectedByLastChildRules);
          }
          if (!isLastChild)
          {
            break;
          }
          AddStyleRelation(checkingContext, element, style::Relation::LastChild);
          return true;
        }
        case SelectorPseudoClass::LastOfType:
        {
          // last-of-type matches the last element of its type
          if (auto *parentElement = DynamicDowncast<dom::Element>(element.ParentNode()))
          {
            auto relation = context.IsSubjectOrAdjacentElement
                              ? style::Relation::ChildrenAffectedByBackwardPositionalRules
                              : style::Relation::DescendantsAffectedByBackwardPositionalRules;
            AddStyleRelation(checkingContext, *parentElement, relation);
            if (!parentElement->IsFinishedParsingChildren())
            {
              return false;
            }
          }
          return IsLastOfType(element, element.TagQualifiedName());
        }
        case SelectorPseudoClass::OnlyChild:
        {
          bool firstChild = IsFirstChildElement(element);
          bool onlyChild = firstChild && IsLastChildElement(element);
          if (auto *parentElement = DynamicDowncast<dom::Element>(element.ParentNode()))
          {
            AddStyleRelation(checkingContext, *parentElement,
                             style::Relation::ChildrenAffectedByFirstChildRules);
            AddStyleRelation(checkingContext, *parentElement,
                             style::Relation::ChildrenAffectedByLastChildRules);
            if (!parentElement->IsFinishedParsingChildren())
            {
              onlyChild = false;
            }
          }
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
        case SelectorPseudoClass::OnlyOfType:
        {
          // FIXME: This selector is very slow.
          if (auto *parentElement = DynamicDowncast<dom::Element>(element.ParentNode()))
          {
            auto forwardRelation = context.IsSubjectOrAdjacentElement
                                     ? style::Relation::ChildrenAffectedByForwardPositionalRules
                                     : style::Relation::DescendantsAffectedByForwardPositionalRules;
            AddStyleRelation(checkingContext, *parentElement, forwardRelation);
            auto backwardRelation = context.IsSubjectOrAdjacentElement
                                      ? style::Relation::ChildrenAffectedByBackwardPositionalRules
                                      : style::Relation::DescendantsAffectedByBackwardPositionalRules;
            AddStyleRelation(checkingContext, *parentElement, backwardRelation);

            if (!parentElement->IsFinishedParsingChildren())
            {
              return false;
            }
          }
          return IsFirstOfType(element, element.TagQualifiedName())
                 && IsLastOfType(element, element.TagQualifiedName());
        }
        case SelectorPseudoClass::Is:
        case SelectorPseudoClass::Where:
        {
          bool hasMatchedAnything = false;

          MatchType localMatchType = MatchType::VirtualPseudoElementOnly;
          krys_debug_assert(selector.SubSelectors());
          for (auto &subselector : *selector.SubSelectors())
          {
            LocalContext subcontext(context);
            subcontext.InFunctionalPseudoClass = true;
            subcontext.PseudoElementEffective = context.PseudoElementEffective;
            subcontext.CurrentSelector = &subselector;
            subcontext.FirstSelectorOfTheFragment = &subselector;
            subcontext.RequestedPseudoElement = null;
            EnumSet<PseudoElementType> localCollectedPseudoElements;
            MatchResult result = MatchRecursively(checkingContext, subcontext, localCollectedPseudoElements);
            context.MatchedHostPseudoClass |= subcontext.MatchedHostPseudoClass;

            // Pseudo elements are not valid inside :is()/:matches()
            // They should also have a specificity of 0 (Selector::SimpleSelectorSpecificity)
            if (localCollectedPseudoElements)
            {
              continue;
            }

            if (result.Outcome == MatchOutcome::SelectorMatches)
            {
              if (result.Type == MatchType::Element)
              {
                localMatchType = MatchType::Element;
              }

              hasMatchedAnything = true;
            }
          }
          if (hasMatchedAnything)
          {
            matchType = localMatchType;
          }
          return hasMatchedAnything;
        }
        case SelectorPseudoClass::Has:
        {
          for (auto &hasSelector : *selector.SubSelectors())
          {
            if (MatchHasPseudoClass(checkingContext, element, hasSelector))
            {
              return true;
            }
          }
          return false;
        }
        case SelectorPseudoClass::PlaceholderShown: return IsPlaceholderShown(element);
        case SelectorPseudoClass::NthChild:
        {
          if (auto *parentElement = DynamicDowncast<dom::Element>(element.ParentNode()))
          {
            auto relation = context.IsSubjectOrAdjacentElement
                              ? style::Relation::ChildrenAffectedByForwardPositionalRules
                              : style::Relation::DescendantsAffectedByForwardPositionalRules;
            AddStyleRelation(checkingContext, *parentElement, relation);
          }

          if (const SelectorList *selectorList = selector.SubSelectors())
          {
            if (!MatchSelectorList(checkingContext, context, element, *selectorList))
            {
              return false;
            }
          }

          int32 count = 1;
          if (const SelectorList *selectorList = selector.SubSelectors())
          {
            for (auto *sibling = dom::TreeQueries::PreviousElementSibling(element); sibling;
                 sibling = dom::TreeQueries::PreviousElementSibling(*sibling))
            {
              if (MatchSelectorList(checkingContext, context, *sibling, *selectorList))
              {
                ++count;
              }
            }
          }
          else
          {
            count += CountElementsBefore(element);
            AddStyleRelation(checkingContext, element, style::Relation::NthChildIndex,
                             static_cast<uint32>(count));
          }

          if (selector.MatchNth(count))
          {
            return true;
          }
          break;
        }
        case SelectorPseudoClass::NthOfType:
        {
          if (auto *parentElement = DynamicDowncast<dom::Element>(element.ParentNode()))
          {
            auto relation = context.IsSubjectOrAdjacentElement
                              ? style::Relation::ChildrenAffectedByForwardPositionalRules
                              : style::Relation::DescendantsAffectedByForwardPositionalRules;
            AddStyleRelation(checkingContext, *parentElement, relation);
          }

          int32 count = 1 + CountElementsOfTypeBefore(element, element.TagQualifiedName());
          if (selector.MatchNth(count))
          {
            return true;
          }
          break;
        }
        case SelectorPseudoClass::NthLastChild:
        {
          if (auto *parentElement = DynamicDowncast<dom::Element>(element.ParentNode()))
          {
            if (const SelectorList *selectorList = selector.SubSelectors())
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
              auto relation = context.IsSubjectOrAdjacentElement
                                ? style::Relation::ChildrenAffectedByBackwardPositionalRules
                                : style::Relation::DescendantsAffectedByBackwardPositionalRules;
              AddStyleRelation(checkingContext, *parentElement, relation);
            }
            if (!parentElement->IsFinishedParsingChildren())
            {
              return false;
            }
          }

          int32 count = 1;
          if (const SelectorList *selectorList = selector.SubSelectors())
          {
            for (auto *sibling = dom::TreeQueries::NextElementSibling(element); sibling;
                 sibling = dom::TreeQueries::NextElementSibling(*sibling))
            {
              if (MatchSelectorList(checkingContext, context, *sibling, *selectorList))
              {
                ++count;
              }
            }
          }
          else
          {
            count += CountElementsAfter(element);
          }

          return selector.MatchNth(count);
        }
        case SelectorPseudoClass::NthLastOfType:
        {
          if (auto *parentElement = DynamicDowncast<dom::Element>(element.ParentNode()))
          {
            auto relation = context.IsSubjectOrAdjacentElement
                              ? style::Relation::ChildrenAffectedByBackwardPositionalRules
                              : style::Relation::DescendantsAffectedByBackwardPositionalRules;
            AddStyleRelation(checkingContext, *parentElement, relation);

            if (!parentElement->IsFinishedParsingChildren())
            {
              return false;
            }
          }
          int32 count = 1 + CountElementsOfTypeAfter(element, element.TagQualifiedName());
          return selector.MatchNth(count);
        }
        case SelectorPseudoClass::Target:
          // FIXME: The document's target element, once documents navigate to fragments.
          break;
        case SelectorPseudoClass::Autofill:
          // FIXME: Autofill.
          return false;
        case SelectorPseudoClass::AnyLink:
        case SelectorPseudoClass::Link:
          // :visited and :link matches are separated later when applying the style. Here both classes match
          // all links...
          return IsLink(element);
        case SelectorPseudoClass::Visited:
        {
          // ...except if :visited matching is disabled for ancestor/sibling matching.
          // Inside functional pseudo class except for :not, :visited never matches.
          if (context.InFunctionalPseudoClass)
          {
            return false;
          }
          auto match = IsLink(element) && context.VisitedType == VisitedMatchType::Enabled;
          if (!match)
          {
            return false;
          }
          // Track that :visited matched during scoping root evaluation
          if (checkingContext.IsEvaluatingScopingRoot)
          {
            checkingContext.ScopingRootMatchesVisited = true;
          }
          return true;
        }
        case SelectorPseudoClass::Focus:        return MatchesFocusPseudoClass(element);
        case SelectorPseudoClass::FocusVisible: return MatchesFocusVisiblePseudoClass(element);
        case SelectorPseudoClass::FocusWithin:  return MatchesFocusWithinPseudoClass(element);
        case SelectorPseudoClass::Hover:
          if (_strictParsing || IsLink(element) || CanMatchHoverOrActiveInQuirksMode(context))
          {
            if (element.Hovered())
            {
              return true;
            }
          }
          break;
        case SelectorPseudoClass::Active:
          if (_strictParsing || IsLink(element) || CanMatchHoverOrActiveInQuirksMode(context))
          {
            if (element.Active())
            {
              return true;
            }
          }
          break;
        case SelectorPseudoClass::Enabled:       return MatchesEnabledPseudoClass(element);
        case SelectorPseudoClass::Default:       return MatchesDefaultPseudoClass(element);
        case SelectorPseudoClass::Disabled:      return MatchesDisabledPseudoClass(element);
        case SelectorPseudoClass::ReadOnly:      return MatchesReadOnlyPseudoClass(element);
        case SelectorPseudoClass::ReadWrite:     return MatchesReadWritePseudoClass(element);
        case SelectorPseudoClass::Optional:      return IsOptionalFormControl(element);
        case SelectorPseudoClass::Required:      return IsRequiredFormControl(element);
        case SelectorPseudoClass::Valid:         return element.MatchesValidPseudoClass();
        case SelectorPseudoClass::Invalid:       return element.MatchesInvalidPseudoClass();
        case SelectorPseudoClass::InRange:       return element.IsInRange();
        case SelectorPseudoClass::OutOfRange:    return element.IsOutOfRange();
        case SelectorPseudoClass::UserValid:     return element.MatchesUserValidPseudoClass();
        case SelectorPseudoClass::UserInvalid:   return element.MatchesUserInvalidPseudoClass();
        case SelectorPseudoClass::Checked:       return IsChecked(element);
        case SelectorPseudoClass::Indeterminate: return MatchesIndeterminatePseudoClass(element);
        case SelectorPseudoClass::Root:
          if (&element == dom::TreeQueries::DocumentElement(element.NodeDocument()))
          {
            return true;
          }
          break;
        case SelectorPseudoClass::Lang:
          krys_debug_assert(selector.LangList() && selector.LangList()->size());
          return MatchesLangPseudoClass(element, *selector.LangList());
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

        case SelectorPseudoClass::Scope:
        {
          // During style invalidation, we don't collect the @scope rules.
          if (checkingContext.ResolvingMode == Mode::StyleInvalidation)
          {
            return !context.IsNegation;
          }

          const dom::Node *contextualReferenceNode =
            !checkingContext.Scope ? dom::TreeQueries::DocumentElement(element.NodeDocument())
                                   : static_cast<const dom::Node *>(checkingContext.Scope.get());
          auto match = &element == contextualReferenceNode;
          if (!match)
          {
            return false;
          }

          if (dom::TreeQueries::GetShadowRoot(element))
          {
            context.MatchedHostPseudoClass = true;
          }

          // Prevent double :visited matching when scoping root matched :visited
          if (checkingContext.ScopingRootMatchesVisited && context.VisitedType == VisitedMatchType::Disabled)
          {
            return false;
          }
          if (checkingContext.ScopingRootMatchesVisited)
          {
            context.VisitedType = VisitedMatchType::Disabled;
          }

          return true;
        }
        case SelectorPseudoClass::State:
          // FIXME: Custom states (ElementInternals.states).
          return false;
        case SelectorPseudoClass::Host:
        {
          if (!context.MustMatchHostPseudoClass)
          {
            return false;
          }
          if (!MatchHostPseudoClass(selector, element, checkingContext))
          {
            return false;
          }
          context.MatchedHostPseudoClass = true;
          return true;
        }
        case SelectorPseudoClass::Defined: return IsDefinedElement(element);

        case SelectorPseudoClass::Dir:     return MatchesDirPseudoClass(element, selector.Argument());

        case SelectorPseudoClass::PopoverOpen:
        case SelectorPseudoClass::Modal:
          // FIXME: Popovers and modal dialogs.
          return false;

        case SelectorPseudoClass::ActiveViewTransition:
        case SelectorPseudoClass::ActiveViewTransitionType:
          // FIXME: View transitions; no document has an active one.
          return false;
      }
      return false;
    }

    if (selector.Match() == SelectorMatch::PseudoElement)
    {
      auto &requestedPseudoElement = checkingContext.RequestedPseudoElement();
      switch (selector.PseudoElement())
      {
        case SelectorPseudoElement::Cue:
        {
          LocalContext subcontext(context);

          for (auto &subselector : *context.CurrentSelector->SubSelectors())
          {
            subcontext.CurrentSelector = &subselector;
            subcontext.FirstSelectorOfTheFragment = &subselector;
            subcontext.InFunctionalPseudoClass = true;
            subcontext.PseudoElementEffective = false;
            EnumSet<PseudoElementType> ignoredPseudoElements;
            if (MatchRecursively(checkingContext, subcontext, ignoredPseudoElements).Outcome
                == MatchOutcome::SelectorMatches)
            {
              return true;
            }
          }
          return false;
        }
        case SelectorPseudoElement::Slotted:
        {
          if (!dom::SlotAlgorithms::GetAssignedSlot(*context.Element))
          {
            return false;
          }
          // ::slotted matches after flattening so it can't match an active <slot>.
          if (Is<html::HTMLSlotElement>(*context.Element)
              && dom::TreeQueries::ContainingShadowRoot(*context.Element))
          {
            return false;
          }
          auto *subselector = context.CurrentSelector->SubSelectors()->FirstSelectorComponent();
          LocalContext subcontext(context);
          subcontext.CurrentSelector = subselector;
          subcontext.FirstSelectorOfTheFragment = subselector;
          subcontext.PseudoElementEffective = false;
          subcontext.InFunctionalPseudoClass = true;
          EnumSet<PseudoElementType> ignoredPseudoElements;
          return MatchRecursively(checkingContext, subcontext, ignoredPseudoElements).Outcome
                 == MatchOutcome::SelectorMatches;
        }
        case SelectorPseudoElement::Part:
        {
          auto appendTranslatedPartNameToRuleScope =
            [&](List<CSSOMStringAtom> &translatedPartNames, CSSOMStringAtom partName)
          {
            if (checkingContext.StyleScopeOrdinal == style::ScopeOrdinal::Element)
            {
              translatedPartNames.push_back(partName);
              return;
            }

            auto *ruleScopeHost =
              style::HostForScopeOrdinal(*context.Element, checkingContext.StyleScopeOrdinal);

            List<CSSOMStringAtom> mappedNames {partName};
            for (auto *shadowRoot = dom::TreeQueries::ContainingShadowRoot(element); shadowRoot;
                 shadowRoot = dom::TreeQueries::ContainingShadowRoot(*shadowRoot->Host()))
            {
              // Apply mappings up to the scope the rules are coming from.
              if (shadowRoot->Host() == ruleScopeHost)
              {
                break;
              }

              // FIXME: The shadow root's part mappings (the host's exportparts attribute); until then no name
              // is exported.
              List<CSSOMStringAtom> newMappedNames;
              mappedNames = newMappedNames;

              if (mappedNames.empty())
              {
                break;
              }
            }
            translatedPartNames.insert(translatedPartNames.end(), mappedNames.begin(), mappedNames.end());
          };

          List<CSSOMStringAtom> translatedPartNames;
          for (auto &partName : PartNames(element))
          {
            appendTranslatedPartNameToRuleScope(translatedPartNames, partName);
          }

          krys_debug_assert(selector.ArgumentList());

          for (auto &part : *selector.ArgumentList())
          {
            if (std::ranges::find(translatedPartNames, part) == translatedPartNames.end())
            {
              return false;
            }
          }
          return true;
        }

        case SelectorPseudoElement::Highlight:
          // Always matches when not specifically requested so it gets added to the collectedPseudoElements.
          if (!requestedPseudoElement)
          {
            return true;
          }
          if (requestedPseudoElement->Type != PseudoElementType::Highlight || !selector.ArgumentList())
          {
            return false;
          }
          return (*selector.ArgumentList())[0] == requestedPseudoElement->NameArgument;

        case SelectorPseudoElement::ViewTransitionGroup:
        case SelectorPseudoElement::ViewTransitionImagePair:
        case SelectorPseudoElement::ViewTransitionOld:
        case SelectorPseudoElement::ViewTransitionNew:
        {
          // Always matches when not specifically requested so it gets added to the collectedPseudoElements.
          if (!requestedPseudoElement)
          {
            return true;
          }
          if (requestedPseudoElement->Type != Selector::StylePseudoElementTypeFor(selector.PseudoElement())
              || !selector.ArgumentList())
          {
            return false;
          }

          auto &list = *selector.ArgumentList();
          auto &name = list[0];
          if (name != dom::StarAtom() && name != requestedPseudoElement->NameArgument)
          {
            return false;
          }

          if (list.size() == 1)
          {
            return true;
          }

          return std::all_of(list.begin() + 1, list.end(),
                             [&](const CSSOMStringAtom &classSelector)
                             {
                               return std::ranges::find(checkingContext.ClassList, classSelector)
                                      != checkingContext.ClassList.end();
                             });
        }

        default: return true;
      }
    }
    return true;
  }

  bool SelectorChecker::MatchSelectorList(CheckingContext &checkingContext, const LocalContext &context,
                                          const dom::Element &element,
                                          const SelectorList &selectorList) const noexcept
  {
    bool hasMatchedAnything = false;

    for (auto &subselector : selectorList)
    {
      LocalContext subcontext(context);
      subcontext.Element = &element;
      subcontext.CurrentSelector = &subselector;
      subcontext.InFunctionalPseudoClass = true;
      subcontext.PseudoElementEffective = false;
      subcontext.FirstSelectorOfTheFragment = &subselector;
      EnumSet<PseudoElementType> ignoredPseudoElements;
      if (MatchRecursively(checkingContext, subcontext, ignoredPseudoElements).Outcome
          == MatchOutcome::SelectorMatches)
      {
        krys_debug_assert(!ignoredPseudoElements);

        hasMatchedAnything = true;
      }
    }
    return hasMatchedAnything;
  }

  bool SelectorChecker::MatchHasPseudoClass(CheckingContext &checkingContext, const dom::Element &element,
                                            const Selector &hasSelector) const noexcept
  {
    // :has() should never be nested with another :has()
    // This is generally discarded at parsing time, but
    // with Nesting some selector can become "contextually invalid".
    if (checkingContext.DisallowHasPseudoClass)
    {
      return false;
    }

    auto matchElement = style::ComputeHasPseudoClassMatchElement(hasSelector);

    auto canMatch = [&]
    {
      switch (matchElement)
      {
        case style::MatchElement::HasChild:
        case style::MatchElement::HasDescendant:
          return dom::TreeQueries::FirstElementChild(element) != nullptr;
        case style::MatchElement::HasSibling:
        case style::MatchElement::HasSiblingDescendant:
          return dom::TreeQueries::NextElementSibling(element) != nullptr;
        default: return true;
      };
    };

    // See if there are any elements that this :has() selector could match.
    if (!canMatch())
    {
      return false;
    }

    auto *cache = checkingContext.SelectorMatchingState
                    ? &checkingContext.SelectorMatchingState->HasPseudoClassMatchCache
                    : nullptr;

    auto cachedMatch = [&](const dom::Element &cachedElement)
    {
      auto it = cache->find(style::MakeHasPseudoClassCacheKey(cachedElement, hasSelector));
      return it == cache->end() ? style::HasPseudoClassMatch::None : it->second;
    };

    auto checkForCachedMatch = [&]() -> Maybe<bool>
    {
      if (!cache)
      {
        return {};
      }
      switch (cachedMatch(element))
      {
        case style::HasPseudoClassMatch::Matches:      return true;
        case style::HasPseudoClassMatch::Fails:
        case style::HasPseudoClassMatch::FailsSubtree: return false;
        case style::HasPseudoClassMatch::None:         break;
      }
      return {};
    };

    // See if we know the result already.
    if (auto match = checkForCachedMatch())
    {
      return *match;
    }

    auto filterForElement = [&]() -> style::HasSelectorFilter *
    {
      if (!checkingContext.SelectorMatchingState)
      {
        return nullptr;
      }
      auto type = style::HasSelectorFilter::TypeForMatchElement(matchElement);
      if (!type)
      {
        return nullptr;
      }
      auto &filtersMap = checkingContext.SelectorMatchingState->HasPseudoClassSelectorFilters;
      auto [it, isNewEntry] = filtersMap.try_emplace(style::MakeHasPseudoClassFilterKey(element, *type));
      // Only build a filter if the same element gets checked second time with a different selector (misses
      // the match cache).
      if (isNewEntry)
      {
        return nullptr;
      }

      if (!it->second)
      {
        it->second = CreateUnique<style::HasSelectorFilter>(element, *type);
      }
      return it->second.get();
    };

    // Check if the bloom filter rejects this selector
    if (auto *filter = filterForElement())
    {
      if (filter->Reject(hasSelector))
      {
        return false;
      }
    }

    CheckingContext hasCheckingContext(Mode::ResolvingStyle);
    hasCheckingContext.Scope = checkingContext.Scope;
    hasCheckingContext.HasScope = &element;
    hasCheckingContext.DisallowHasPseudoClass = true;

    bool matchedInsideScope = false;

    auto checkRelative = [&](const dom::Element &elementToCheck)
    {
      LocalContext hasContext(hasSelector, elementToCheck, VisitedMatchType::Disabled, null);
      hasContext.InFunctionalPseudoClass = true;
      hasContext.PseudoElementEffective = false;

      EnumSet<PseudoElementType> ignoredPseudoElements;
      auto result = MatchRecursively(hasCheckingContext, hasContext, ignoredPseudoElements).Outcome
                    == MatchOutcome::SelectorMatches;

      if (hasCheckingContext.MatchedInsideScope)
      {
        matchedInsideScope = true;
      }

      return result;
    };

    auto checkDescendants = [&](const dom::Element &descendantRoot)
    {
      for (auto *node = dom::TreeQueries::Next(descendantRoot, &descendantRoot); node;)
      {
        auto *descendant = DynamicDowncast<dom::Element>(*node);
        if (!descendant)
        {
          node = dom::TreeQueries::Next(*node, &descendantRoot);
          continue;
        }

        if (checkRelative(*descendant))
        {
          return true;
        }

        if (cache && dom::TreeQueries::FirstElementChild(*descendant))
        {
          if (cachedMatch(*descendant) == style::HasPseudoClassMatch::FailsSubtree)
          {
            node = dom::TreeQueries::NextSkippingChildren(*descendant, &descendantRoot);
            continue;
          }
        }

        node = dom::TreeQueries::Next(*descendant, &descendantRoot);
      }

      return false;
    };

    auto match = [&]
    {
      switch (matchElement)
      {
        // :has(> .child)
        case style::MatchElement::HasChild:
          for (auto *child = dom::TreeQueries::FirstElementChild(element); child;
               child = dom::TreeQueries::NextElementSibling(*child))
          {
            if (checkRelative(*child))
            {
              return true;
            }
          }
          break;
        // :has(.descendant)
        case style::MatchElement::HasDescendant:
        {
          if (cache)
          {
            // See if we already know this descendant selector doesn't match in this subtree.
            for (auto *ancestor = element.ParentElement(); ancestor; ancestor = ancestor->ParentElement())
            {
              if (cachedMatch(*ancestor) == style::HasPseudoClassMatch::FailsSubtree)
              {
                return false;
              }
            }
          }
          if (checkDescendants(element))
          {
            return true;
          }

          break;
        }
        // FIXME: Add a separate case for adjacent combinator.
        // :has(+ .sibling)
        // :has(~ .sibling)
        case style::MatchElement::HasSibling:
          for (auto *sibling = dom::TreeQueries::NextElementSibling(element); sibling;
               sibling = dom::TreeQueries::NextElementSibling(*sibling))
          {
            if (checkRelative(*sibling))
            {
              return true;
            }
          }
          break;
        // FIXME: Add a separate case for adjacent combinator.
        // :has(+ .sibling .descendant)
        // :has(~ .sibling .descendant)
        case style::MatchElement::HasSiblingDescendant:
          for (auto *sibling = dom::TreeQueries::NextElementSibling(element); sibling;
               sibling = dom::TreeQueries::NextElementSibling(*sibling))
          {
            if (checkDescendants(*sibling))
            {
              return true;
            }
          }
          break;

        default: krys_debug_assert(false); break;
      }
      return false;
    };

    auto result = match();

    auto matchTypeForCache = [&]
    {
      if (result)
      {
        return style::HasPseudoClassMatch::Matches;
      }
      if (matchedInsideScope)
      {
        return style::HasPseudoClassMatch::Fails;
      }
      return style::HasPseudoClassMatch::FailsSubtree;
    };

    if (cache)
    {
      cache->try_emplace(style::MakeHasPseudoClassCacheKey(element, hasSelector), matchTypeForCache());
    }

    auto forwardStyleRelation = [&](const style::Relation &relation)
    {
      switch (relation.RelationType)
      {
        case style::Relation::ChildrenAffectedByForwardPositionalRules:
        case style::Relation::ChildrenAffectedByBackwardPositionalRules:
          checkingContext.StyleRelations.emplace_back(
            *relation.Element, style::Relation::AffectedByHasWithPositionalPseudoClass);
          return;
        case style::Relation::ChildrenAffectedByFirstChildRules:
        case style::Relation::ChildrenAffectedByLastChildRules:
          checkingContext.StyleRelations.push_back(relation);
          return;
        case style::Relation::AffectedByEmpty:
        case style::Relation::AffectedByPreviousSibling:
        case style::Relation::DescendantsAffectedByPreviousSibling:
        case style::Relation::AffectsNextSibling:
        case style::Relation::DescendantsAffectedByForwardPositionalRules:
        case style::Relation::DescendantsAffectedByBackwardPositionalRules:
        case style::Relation::FirstChild:
        case style::Relation::LastChild:
        case style::Relation::NthChildIndex:                                return;
        case style::Relation::AffectedByHasWithPositionalPseudoClass:       krys_debug_assert(false); return;
      }
    };

    for (auto &relation : hasCheckingContext.StyleRelations)
    {
      forwardStyleRelation(relation);
    }

    return result;
  }

  bool SelectorChecker::CheckViewTransitionPseudoClass(const CheckingContext &checkingContext,
                                                       const dom::Element &element,
                                                       const Selector &selector) const noexcept
  {
    (void)element;
    krys_debug_assert(selector.Match() == SelectorMatch::PseudoClass);

    if (selector.PseudoClass() != SelectorPseudoClass::OnlyChild)
    {
      return false;
    }

    auto &pseudoIdentifier = checkingContext.RequestedPseudoElement();
    if (!pseudoIdentifier)
    {
      return false;
    }

    switch (pseudoIdentifier->Type)
    {
      case PseudoElementType::ViewTransitionImagePair: return true;
      // FIXME: The group, old and new pseudo-elements depend on the document's active view transition.
      default:                                         return false;
    }
  }

  uint32 SelectorChecker::DetermineLinkMatchType(const Selector *selector,
                                                 const ScopeRule *scopeRule) noexcept
  {
    uint32 linkMatchType = MatchAll;

    auto scopingRootLinkMatchType = [&]() -> uint32
    {
      if (!scopeRule || scopeRule->ScopeStart().IsEmpty())
      {
        return MatchAll;
      }

      // FIXME: Support multiple selectors
      auto scopingRootLinkMatchType =
        DetermineLinkMatchType(scopeRule->ScopeStart().FirstSelectorComponent());
      if (!scopeRule->ScopeEnd().IsEmpty())
      {
        scopingRootLinkMatchType &= DetermineLinkMatchType(scopeRule->ScopeEnd().FirstSelectorComponent());
      }
      return scopingRootLinkMatchType;
    };

    // Statically determine if this selector will match a link in visited, unvisited or any state, or never.
    // :visited never matches other elements than the innermost link element.
    for (; selector; selector = selector->PrecedingComplexSelectorComponent())
    {
      if (selector->Match() == SelectorMatch::PseudoClass)
      {
        switch (selector->PseudoClass())
        {
          case SelectorPseudoClass::Link:    linkMatchType &= ~MatchVisited; break;
          case SelectorPseudoClass::Visited: linkMatchType &= ~MatchLink; break;
          case SelectorPseudoClass::Scope:
            // :scope refers to the scoping root, so it should inherit the link matching constraints from it.
            linkMatchType &= scopingRootLinkMatchType();
            break;
          default: break;
        }
      }
      auto relation = selector->Relation();
      if (relation == SelectorRelation::Compounding)
      {
        continue;
      }
      if (!selector->HasDescendantOrChildRelation())
      {
        return linkMatchType;
      }
      if (linkMatchType != MatchAll)
      {
        return linkMatchType;
      }
    }
    return linkMatchType;
  }
}
