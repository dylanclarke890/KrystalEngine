#include "Krystal.Booey/HTML/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/HTMLLegendElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"
#include <algorithm>

namespace krys::boo::html
{
  namespace
  {
    void UpdateFromControlElementsAncestorDisabledStateUnder(HTMLElement &startNode, bool isDisabled) noexcept
    {
      for (dom::Node *node = &startNode; node;)
      {
        auto *element = DynamicDowncast<dom::Element>(*node);
        if (!element)
        {
          node = dom::TreeQueries::Next(*node, &startNode);
          continue;
        }

        if (auto *htmlElement = DynamicDowncast<HTMLElement>(*element))
        {
          if (auto *listedElement = htmlElement->AsValidatedFormListedElement())
          {
            listedElement->SetDisabledByAncestorFieldset(isDisabled);
          }
        }

        // Don't call SetDisabledByAncestorFieldset() on form controls inside disabled fieldsets.
        if (Is<HTMLFieldSetElement>(*element) && element->HasAttribute(u8"disabled"))
        {
          node = dom::TreeQueries::NextSkippingChildren(*node, &startNode);
        }
        else
        {
          node = dom::TreeQueries::Next(*node, &startNode);
        }
      }
    }
  }

  HTMLFieldSetElement::HTMLFieldSetElement(dom::Document &document) noexcept
      : HTMLFormControlElement(document, HTMLElementInterface::FieldSet)
  {
  }

  HTMLFieldSetElement::~HTMLFieldSetElement() noexcept = default;

  bool HTMLFieldSetElement::IsDisabledFormControl() const noexcept
  {
    // The fieldset element itself should never be considered disabled, it is only supposed to affect its
    // descendants: https://html.spec.whatwg.org/#concept-fe-disabled
    return false;
  }

  // https://html.spec.whatwg.org/#concept-element-disabled
  // https://html.spec.whatwg.org/#concept-fieldset-disabled
  bool HTMLFieldSetElement::IsActuallyDisabled() const noexcept
  {
    return HTMLFormControlElement::IsDisabledFormControl();
  }

  void HTMLFieldSetElement::DisabledStateChanged() noexcept
  {
    // This element must be updated before the style of nodes in its subtree gets recalculated.
    HTMLFormControlElement::DisabledStateChanged();

    if (DisabledByAncestorFieldset())
    {
      return;
    }

    bool thisFieldsetIsDisabled = HasAttribute(u8"disabled");
    bool hasSeenFirstLegendElement = false;
    for (auto *child = dom::TreeQueries::FirstElementChild(*this); child;
         child = dom::TreeQueries::NextElementSibling(*child))
    {
      auto *control = DynamicDowncast<HTMLElement>(*child);
      if (!control)
      {
        continue;
      }
      if (!hasSeenFirstLegendElement && Is<HTMLLegendElement>(*control))
      {
        hasSeenFirstLegendElement = true;
        UpdateFromControlElementsAncestorDisabledStateUnder(*control, false /* isDisabled */);
        continue;
      }
      UpdateFromControlElementsAncestorDisabledStateUnder(*control, thisFieldsetIsDisabled);
    }
  }

  void HTMLFieldSetElement::OnChildrenChanged() noexcept
  {
    HTMLFormControlElement::OnChildrenChanged();
    if (!HasAttribute(u8"disabled"))
    {
      return;
    }

    auto *legend = Legend();
    if (!legend)
    {
      return;
    }

    // We only care about the first legend element (in which form controls are not disabled by this element)
    // changing here.
    UpdateFromControlElementsAncestorDisabledStateUnder(*legend, false /* isDisabled */);
    for (auto *sibling = dom::TreeQueries::NextElementSibling(*legend); sibling;
         sibling = dom::TreeQueries::NextElementSibling(*sibling))
    {
      if (auto *otherLegend = DynamicDowncast<HTMLLegendElement>(*sibling))
      {
        UpdateFromControlElementsAncestorDisabledStateUnder(*otherLegend, true);
      }
    }
  }

  bool HTMLFieldSetElement::MatchesValidPseudoClass() const noexcept
  {
    return std::ranges::none_of(_invalidDescendants, [](auto &element) { return element.get() != nullptr; });
  }

  bool HTMLFieldSetElement::MatchesInvalidPseudoClass() const noexcept
  {
    return !MatchesValidPseudoClass();
  }

  RenderPtr<RenderElement> HTMLFieldSetElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    // Fieldsets should make a block flow if display: inline or table types are set.
    return RenderElement::CreateFor(*this, krys::move(style),
                                    {RenderElement::ConstructBlockLevelRendererFor::Inline,
                                     RenderElement::ConstructBlockLevelRendererFor::TableOrTablePart});
  }

  HTMLLegendElement *HTMLFieldSetElement::Legend() const noexcept
  {
    for (auto *child = dom::TreeQueries::FirstElementChild(*this); child;
         child = dom::TreeQueries::NextElementSibling(*child))
    {
      if (auto *legend = DynamicDowncast<HTMLLegendElement>(*child))
      {
        return const_cast<HTMLLegendElement *>(legend);
      }
    }
    return nullptr;
  }

  Ref<dom::HTMLCollection> HTMLFieldSetElement::Elements() noexcept
  {
    return CreateRef<dom::LiveHTMLCollection>(*this, [](const dom::Element &element)
                                              { return element.IsFormListedElement(); });
  }

  void HTMLFieldSetElement::AddInvalidDescendant(const HTMLElement &invalidFormControlElement) noexcept
  {
    krys_debug_assert(!Is<HTMLFieldSetElement>(invalidFormControlElement));
    krys_debug_assert(invalidFormControlElement.MatchesInvalidPseudoClass());

    // An element can be added more than once, as its fieldset ancestors are found from its parent, not from
    // where its subtree was inserted.
    if (std::ranges::any_of(_invalidDescendants,
                            [&](auto &element) { return element.get() == &invalidFormControlElement; }))
    {
      return;
    }

    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (MatchesValidPseudoClass())
    {
      styleInvalidation.emplace(*this, std::initializer_list<Pair<css::SelectorPseudoClass, bool>> {
                                         {css::SelectorPseudoClass::Valid, false},
                                         {css::SelectorPseudoClass::Invalid, true},
                                       });
    }

    _invalidDescendants.push_back(CreateWeakPtr(&invalidFormControlElement));
  }

  void HTMLFieldSetElement::RemoveInvalidDescendant(const HTMLElement &formControlElement) noexcept
  {
    krys_debug_assert(!Is<HTMLFieldSetElement>(formControlElement));

    std::erase_if(_invalidDescendants, [](auto &element) { return !element.get(); });
    auto position = std::ranges::find_if(_invalidDescendants,
                                         [&](auto &element) { return element.get() == &formControlElement; });
    if (position == _invalidDescendants.end())
    {
      return;
    }

    Maybe<style::PseudoClassChangeInvalidation> styleInvalidation;
    if (_invalidDescendants.size() == 1)
    {
      styleInvalidation.emplace(*this, std::initializer_list<Pair<css::SelectorPseudoClass, bool>> {
                                         {css::SelectorPseudoClass::Valid, true},
                                         {css::SelectorPseudoClass::Invalid, false},
                                       });
    }

    _invalidDescendants.erase(position);
  }

#pragma region HTMLFieldSetElement

  bool HTMLFieldSetElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLFieldSetElement::Disabled(bool value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::DOMString HTMLFieldSetElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLFieldSetElement::Name(dom::DOMString &&value) noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

#pragma endregion
}
