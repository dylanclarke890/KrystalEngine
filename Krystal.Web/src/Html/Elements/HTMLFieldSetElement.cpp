#include "Krystal.Web/Html/Elements/HTMLFieldSetElement.hpp"
#include "Krystal.Web/Dom/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Web/Dom/Traversal/TreeTraversal.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Web/Html/Elements/HTMLLegendElement.hpp"
#include "Krystal.Web/Rendering/RenderElement.hpp"
#include "Krystal.Web/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include <algorithm>

namespace krys::web
{
  namespace
  {
    void UpdateFromControlElementsAncestorDisabledStateUnder(HTMLElement &startNode, bool isDisabled) noexcept
    {
      for (Node *node = &startNode; node;)
      {
        auto *element = DynamicDowncast<web::Element>(*node);
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

        // A disabled fieldset's subtree follows that fieldset instead.
        if (Is<HTMLFieldSetElement>(*element) && element->HasAttribute(u8"disabled"))
        {
          node = dom::TreeTraversal::NextSkippingChildren(*node, &startNode);
        }
        else
        {
          node = dom::TreeQueries::Next(*node, &startNode);
        }
      }
    }
  }

  HTMLFieldSetElement::HTMLFieldSetElement(Document &document) noexcept
      : HTMLFormControlElement(document, html::HTMLElementInterface::FieldSet)
  {
  }

  HTMLFieldSetElement::~HTMLFieldSetElement() noexcept = default;

  bool HTMLFieldSetElement::IsDisabledFormControl() const noexcept
  {
    // The fieldset element itself should never be considered disabled, it is only supposed to affect its
    // descendants: https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#concept-fe-disabled
    return false;
  }

  // https://html.spec.whatwg.org/multipage/semantics-other.html#concept-element-disabled
  // https://html.spec.whatwg.org/multipage/form-elements.html#concept-fieldset-disabled
  bool HTMLFieldSetElement::IsActuallyDisabled() const noexcept
  {
    return HTMLFormControlElement::IsDisabledFormControl();
  }

  void HTMLFieldSetElement::DisabledStateChanged() noexcept
  {
    // The fieldset's own state is updated before its descendants restyle.
    HTMLFormControlElement::DisabledStateChanged();

    if (DisabledByAncestorFieldset())
    {
      return;
    }

    bool thisFieldsetIsDisabled = HasAttribute(u8"disabled");
    bool hasSeenFirstLegendElement = false;
    for (auto *child = dom::TreeTraversal::FirstElementChild(*this); child;
         child = dom::TreeTraversal::NextElementSibling(*child))
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
    for (auto *sibling = dom::TreeTraversal::NextElementSibling(*legend); sibling;
         sibling = dom::TreeTraversal::NextElementSibling(*sibling))
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
    // An inline or table display still gives a fieldset a block container.
    return RenderElement::CreateFor(*this, krys::move(style),
                                    {RenderElement::ConstructBlockLevelRendererFor::Inline,
                                     RenderElement::ConstructBlockLevelRendererFor::TableOrTablePart});
  }

  HTMLLegendElement *HTMLFieldSetElement::Legend() const noexcept
  {
    for (auto *child = dom::TreeTraversal::FirstElementChild(*this); child;
         child = dom::TreeTraversal::NextElementSibling(*child))
    {
      if (auto *legend = DynamicDowncast<HTMLLegendElement>(*child))
      {
        return const_cast<HTMLLegendElement *>(legend);
      }
    }
    return nullptr;
  }

  Ref<HTMLCollection> HTMLFieldSetElement::Elements() noexcept
  {
    if (auto *elements = _elements.get())
    {
      return ShareRef(*elements);
    }
    // The elements IDL attribute must return an HTMLCollection rooted at the fieldset element, whose filter
    // matches listed elements.
    Ref<HTMLCollection> elements = CreateRef<LiveHTMLCollection>(*this, [](const web::Element &element)
                                                                 { return element.IsFormListedElement(); });
    _elements = CreateWeakPtr<HTMLCollection>(elements.get());
    return elements;
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
    return html::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLFieldSetElement::Disabled(bool value) noexcept
  {
    html::CEReactionsScope ceReactions;
    html::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  DOMString HTMLFieldSetElement::Name() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLFieldSetElement::Name(DOMString value) noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name", krys::move(value));
  }

#pragma endregion
}
