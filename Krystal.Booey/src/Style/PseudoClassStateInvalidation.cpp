#include "Krystal.Booey/Style/PseudoClassStateInvalidation.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include <initializer_list>

namespace krys::boo::style
{
  namespace
  {
    using PseudoClass = css::SelectorPseudoClass;

    KRYS_NODISCARD bool IsHTMLElement(const dom::Element &element, html::TagName tagName) noexcept
    {
      return Is<html::HTMLElement>(element) && element.TagQualifiedName().TagName() == tagName;
    }

    KRYS_NODISCARD bool IsOneOf(const dom::Element &element,
                                std::initializer_list<html::TagName> tagNames) noexcept
    {
      return Is<html::HTMLElement>(element)
             && std::ranges::find(tagNames, element.TagQualifiedName().TagName()) != tagNames.end();
    }

    /// @brief Whether @p element is in the state @p pseudoClass matches.
    KRYS_NODISCARD bool MatchesState(const dom::Element &element, PseudoClass pseudoClass) noexcept
    {
      switch (pseudoClass)
      {
        case PseudoClass::AnyLink:
        case PseudoClass::Link:          return css::IsLink(element);
        case PseudoClass::Indeterminate: return css::MatchesIndeterminatePseudoClass(element);
        case PseudoClass::Disabled:      return css::MatchesDisabledPseudoClass(element);
        case PseudoClass::Enabled:       return css::MatchesEnabledPseudoClass(element);
        case PseudoClass::ReadOnly:      return css::MatchesReadOnlyPseudoClass(element);
        case PseudoClass::ReadWrite:     return css::MatchesReadWritePseudoClass(element);
        default:                         krys_unreachable();
      }
    }

    /// @brief Starts the invalidations for states of @p element that may flip: each as if it flips, which
    /// costs a little extra invalidation when it doesn't.
    void InvalidateStates(PseudoClassChangeInvalidations &invalidations, dom::Element &element,
                          std::initializer_list<PseudoClass> pseudoClasses) noexcept
    {
      if (!element.NeedsStyleInvalidation())
      {
        return;
      }
      for (auto pseudoClass : pseudoClasses)
      {
        invalidations.push_back(CreateUnique<PseudoClassChangeInvalidation>(
          element, pseudoClass, !MatchesState(element, pseudoClass)));
      }
    }

    /// @brief Starts the invalidation for a functional pseudo-class (`:lang()`, `:dir()`) whose matches may
    /// change either way.
    void InvalidateAnyValue(PseudoClassChangeInvalidations &invalidations, dom::Element &element,
                            PseudoClass pseudoClass) noexcept
    {
      if (!element.NeedsStyleInvalidation())
      {
        return;
      }
      invalidations.push_back(CreateUnique<PseudoClassChangeInvalidation>(
        element, pseudoClass, PseudoClassChangeInvalidation::AnyValue));
    }

    /// @brief Calls @p function for the descendants of @p element that inherit a state from it, crossing into
    /// shadow trees if @p crossShadowBoundaries: the subtrees of descendants for which @p stop is true are
    /// skipped.
    template <typename TStop, typename TFunction>
    void ForEachInheritingDescendant(dom::Element &element, bool crossShadowBoundaries, TStop &&stop,
                                     TFunction &&function) noexcept
    {
      auto visitChildren = [&](this auto &self, dom::ContainerNode &container) -> void
      {
        for (auto *child = dom::TreeQueries::FirstElementChild(container); child;
             child = dom::TreeQueries::NextElementSibling(*child))
        {
          if (stop(*child))
          {
            continue;
          }
          function(*child);
          if (auto *shadowRoot = dom::TreeQueries::GetShadowRoot(*child); shadowRoot && crossShadowBoundaries)
          {
            self(*shadowRoot);
          }
          self(*child);
        }
      };

      if (auto *shadowRoot = dom::TreeQueries::GetShadowRoot(element); shadowRoot && crossShadowBoundaries)
      {
        visitChildren(*shadowRoot);
      }
      visitChildren(element);
    }

    KRYS_NODISCARD bool HasLanguageAttribute(const dom::Element &element) noexcept
    {
      return element.HasAttributeNS(dom::DOMStringAtom(infra::Namespaces::XML), u8"lang")
             || element.HasAttributeNS(dom::DOMStringAtom::Null(), u8"lang");
    }

    KRYS_NODISCARD bool HasDirectionality(const dom::Element &element) noexcept
    {
      if (!Is<html::HTMLElement>(element))
      {
        return false;
      }
      auto dir =
        text::ToASCIILower(dom::DOMStringView(element.GetAttribute(u8"dir").value_or(dom::DOMString())));
      return dir == u8"ltr" || dir == u8"rtl" || dir == u8"auto";
    }

    KRYS_NODISCARD bool HasContentEditableAttribute(const dom::Element &element) noexcept
    {
      return Is<html::HTMLElement>(element) && element.HasAttribute(u8"contenteditable");
    }

    void InvalidateDisabledStates(PseudoClassChangeInvalidations &invalidations,
                                  dom::Element &element) noexcept
    {
      InvalidateStates(invalidations, element, {PseudoClass::Disabled, PseudoClass::Enabled});
    }
  }

  void InvalidatePseudoClassesForAttributeChange(PseudoClassChangeInvalidations &invalidations,
                                                 dom::Element &element,
                                                 const dom::QualifiedName &attributeName,
                                                 Maybe<dom::DOMStringView> oldValue,
                                                 Maybe<dom::DOMStringView> newValue) noexcept
  {
    if (!element.NeedsStyleInvalidation())
    {
      return;
    }

    auto localName = attributeName.LocalName().View();
    auto &namespaceURI = attributeName.NamespaceURI();
    bool presenceChanged = oldValue.has_value() != newValue.has_value();

    if (namespaceURI.View() == infra::Namespaces::XML)
    {
      if (localName == u8"lang")
      {
        InvalidateAnyValue(invalidations, element, PseudoClass::Lang);
        ForEachInheritingDescendant(element, true, HasLanguageAttribute, [&](dom::Element &descendant)
                                    { InvalidateAnyValue(invalidations, descendant, PseudoClass::Lang); });
      }
      return;
    }

    if (namespaceURI.View() == infra::Namespaces::XLink)
    {
      if (localName == u8"href" && presenceChanged && element.NamespaceURI().View() == infra::Namespaces::SVG
          && element.LocalName().View() == u8"a")
      {
        InvalidateStates(invalidations, element, {PseudoClass::AnyLink, PseudoClass::Link});
      }
      return;
    }

    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }

    if (localName == u8"lang")
    {
      InvalidateAnyValue(invalidations, element, PseudoClass::Lang);
      ForEachInheritingDescendant(element, true, HasLanguageAttribute, [&](dom::Element &descendant)
                                  { InvalidateAnyValue(invalidations, descendant, PseudoClass::Lang); });
      return;
    }

    if (localName == u8"href")
    {
      bool isLinkElement =
        IsOneOf(element, {html::TagName::a, html::TagName::area})
        || (element.NamespaceURI().View() == infra::Namespaces::SVG && element.LocalName().View() == u8"a");
      if (presenceChanged && isLinkElement)
      {
        InvalidateStates(invalidations, element, {PseudoClass::AnyLink, PseudoClass::Link});
      }
      return;
    }

    if (!Is<html::HTMLElement>(element))
    {
      return;
    }

    if (localName == u8"dir")
    {
      InvalidateAnyValue(invalidations, element, PseudoClass::Dir);
      ForEachInheritingDescendant(element, true, HasDirectionality, [&](dom::Element &descendant)
                                  { InvalidateAnyValue(invalidations, descendant, PseudoClass::Dir); });
      return;
    }

    if (localName == u8"contenteditable")
    {
      InvalidateStates(invalidations, element, {PseudoClass::ReadOnly, PseudoClass::ReadWrite});
      ForEachInheritingDescendant(
        element, false, HasContentEditableAttribute, [&](dom::Element &descendant)
        { InvalidateStates(invalidations, descendant, {PseudoClass::ReadOnly, PseudoClass::ReadWrite}); });
      return;
    }

    auto tagName = element.TagQualifiedName().TagName();
    if (localName == u8"type")
    {
      if (tagName == html::TagName::input)
      {
        // Telephone inputs are left-to-right.
        InvalidateAnyValue(invalidations, element, PseudoClass::Dir);
      }
      return;
    }

    if (localName == u8"disabled")
    {
      if (!presenceChanged)
      {
        return;
      }
      switch (tagName)
      {
        case html::TagName::option: InvalidateDisabledStates(invalidations, element); break;
        case html::TagName::optgroup:
          InvalidateDisabledStates(invalidations, element);
          for (auto *child = dom::TreeQueries::FirstElementChild(element); child;
               child = dom::TreeQueries::NextElementSibling(*child))
          {
            if (IsHTMLElement(*child, html::TagName::option))
            {
              InvalidateDisabledStates(invalidations, *child);
            }
          }
          break;
        default: break;
      }
      return;
    }

    if (tagName == html::TagName::progress && localName == u8"value" && presenceChanged)
    {
      InvalidateStates(invalidations, element, {PseudoClass::Indeterminate});
    }
  }
}
