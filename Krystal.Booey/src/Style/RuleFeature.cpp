#include "Krystal.Booey/Style/RuleFeature.hpp"
#include "Krystal.Booey/CSS/Selectors/Selector.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::style
{
  namespace
  {
    KRYS_NODISCARD bool IsSiblingOrSubject(MatchElement matchElement) noexcept
    {
      switch (matchElement)
      {
        case MatchElement::Subject:
        case MatchElement::IndirectSibling:
        case MatchElement::DirectSibling:
        case MatchElement::AnySibling:
        case MatchElement::HasSibling:
        case MatchElement::HasAnySibling:
        case MatchElement::Host:
        case MatchElement::HostChild:            return true;
        case MatchElement::Parent:
        case MatchElement::Ancestor:
        case MatchElement::ParentSibling:
        case MatchElement::AncestorSibling:
        case MatchElement::ParentAnySibling:
        case MatchElement::AncestorAnySibling:
        case MatchElement::HasChild:
        case MatchElement::HasDescendant:
        case MatchElement::HasSiblingDescendant:
        case MatchElement::HasChildParent:
        case MatchElement::HasChildAncestor:
        case MatchElement::HasDescendantParent:
        case MatchElement::HasNonSubject:
        case MatchElement::HasScopeBreaking:     return false;
      }
      krys_debug_assert(false);
      return false;
    }

    KRYS_NODISCARD MatchElement ComputeNextMatchElement(MatchElement matchElement,
                                                        css::SelectorRelation relation) noexcept
    {
      krys_debug_assert(!IsHasPseudoClassMatchElement(matchElement));

      if (IsSiblingOrSubject(matchElement))
      {
        switch (relation)
        {
          case css::SelectorRelation::Compounding: return matchElement;
          case css::SelectorRelation::Descendant:  return MatchElement::Ancestor;
          case css::SelectorRelation::Child:       return MatchElement::Parent;
          case css::SelectorRelation::SubsequentSibling:
            if (matchElement == MatchElement::AnySibling)
            {
              return MatchElement::AnySibling;
            }
            return MatchElement::IndirectSibling;
          case css::SelectorRelation::NextSibling:
            if (matchElement == MatchElement::AnySibling)
            {
              return MatchElement::AnySibling;
            }
            return matchElement == MatchElement::Subject ? MatchElement::DirectSibling
                                                         : MatchElement::IndirectSibling;
          case css::SelectorRelation::ShadowDescendant:
          case css::SelectorRelation::ShadowPartDescendant: return MatchElement::Host;
          case css::SelectorRelation::ShadowSlotted:        return MatchElement::HostChild;
        }
      }
      switch (relation)
      {
        case css::SelectorRelation::Compounding: return matchElement;
        case css::SelectorRelation::Descendant:
        case css::SelectorRelation::Child:       return MatchElement::Ancestor;
        case css::SelectorRelation::SubsequentSibling:
        case css::SelectorRelation::NextSibling:
          return matchElement == MatchElement::Parent ? MatchElement::ParentSibling
                                                      : MatchElement::AncestorSibling;
        case css::SelectorRelation::ShadowDescendant:
        case css::SelectorRelation::ShadowPartDescendant: return MatchElement::Host;
        case css::SelectorRelation::ShadowSlotted:        return MatchElement::HostChild;
      }
      krys_debug_assert(false);
      return matchElement;
    }
  }

  bool IsHasPseudoClassMatchElement(MatchElement matchElement) noexcept
  {
    switch (matchElement)
    {
      case MatchElement::HasChild:
      case MatchElement::HasDescendant:
      case MatchElement::HasSibling:
      case MatchElement::HasSiblingDescendant:
      case MatchElement::HasAnySibling:
      case MatchElement::HasNonSubject:
      case MatchElement::HasScopeBreaking:     return true;
      default:                                 return false;
    }
  }

  MatchElement ComputeHasPseudoClassMatchElement(const css::Selector &hasSelector) noexcept
  {
    auto hasMatchElement = MatchElement::Subject;
    for (auto *simpleSelector = &hasSelector; simpleSelector->PrecedingComplexSelectorComponent();
         simpleSelector = simpleSelector->PrecedingComplexSelectorComponent())
    {
      hasMatchElement = ComputeNextMatchElement(hasMatchElement, simpleSelector->Relation());
    }

    switch (hasMatchElement)
    {
      case MatchElement::Parent:
      case MatchElement::Subject:              return MatchElement::HasChild;
      case MatchElement::Ancestor:             return MatchElement::HasDescendant;
      case MatchElement::IndirectSibling:
      case MatchElement::DirectSibling:
      case MatchElement::AnySibling:           return MatchElement::HasSibling;
      case MatchElement::ParentSibling:
      case MatchElement::AncestorSibling:
      case MatchElement::ParentAnySibling:
      case MatchElement::AncestorAnySibling:   return MatchElement::HasSiblingDescendant;
      case MatchElement::HasChild:
      case MatchElement::HasDescendant:
      case MatchElement::HasSibling:
      case MatchElement::HasSiblingDescendant:
      case MatchElement::HasAnySibling:
      case MatchElement::HasChildParent:
      case MatchElement::HasChildAncestor:
      case MatchElement::HasDescendantParent:
      case MatchElement::HasNonSubject:
      case MatchElement::HasScopeBreaking:
      case MatchElement::Host:
      case MatchElement::HostChild:            krys_debug_assert(false); break;
    }
    return MatchElement::HasChild;
  }
}
