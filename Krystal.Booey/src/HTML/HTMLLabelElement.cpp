#include "Krystal.Booey/HTML/HTMLLabelElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/ValidatedFormListedElement.hpp"

namespace krys::boo::html
{
  namespace
  {
    /// @brief The labelable element with the ID @p id in @p label's tree.
    KRYS_NODISCARD HTMLElement *ElementForAttributeIfLabelable(const HTMLLabelElement &label,
                                                               dom::DOMStringView id) noexcept
    {
      auto &root = dom::TreeQueries::Root(const_cast<HTMLLabelElement &>(label));
      RefPtr<dom::Element> element;
      if (auto *document = DynamicDowncast<dom::Document>(root))
      {
        element = document->GetElementById(id);
      }
      else if (auto *fragment = DynamicDowncast<dom::DocumentFragment>(root))
      {
        element = fragment->GetElementById(id);
      }

      auto *labelableElement = DynamicDowncast<HTMLElement>(element.get());
      return labelableElement && labelableElement->IsLabelable() ? labelableElement : nullptr;
    }
  }

  HTMLLabelElement::HTMLLabelElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Label)
  {
  }

#pragma region HTMLLabelElement

  dom::DOMString HTMLLabelElement::HtmlFor() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"for").value_or(u8"");
  }

  void HTMLLabelElement::HtmlFor(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"for", krys::move(value));
  }

  RefPtr<HTMLElement> HTMLLabelElement::Control() const noexcept
  {
    auto forAttribute = GetAttribute(u8"for");
    if (!forAttribute)
    {
      // https://html.spec.whatwg.org/#labeled-control
      for (auto *node = dom::TreeQueries::Next(*this, this); node; node = dom::TreeQueries::Next(*node, this))
      {
        if (auto *descendant = DynamicDowncast<HTMLElement>(*node); descendant && descendant->IsLabelable())
        {
          return ShareRefPtr(descendant);
        }
      }
      return nullptr;
    }

    return IsConnected() ? ShareRefPtr(ElementForAttributeIfLabelable(*this, *forAttribute)) : nullptr;
  }

  RefPtr<HTMLElement> HTMLLabelElement::ControlForBindings() const noexcept
  {
    // FIXME: Retargeting against the label's tree, once there are reference targets.
    return Control();
  }

  HTMLFormElement *HTMLLabelElement::Form() const noexcept
  {
    if (auto element = Control())
    {
      if (auto *listedElement = element->AsValidatedFormListedElement())
      {
        return listedElement->Form();
      }
    }
    return nullptr;
  }

  RefPtr<HTMLFormElement> HTMLLabelElement::FormForBindings() const noexcept
  {
    // FIXME: Retargeting against the label's tree, once there are reference targets.
    return ShareRefPtr(Form());
  }

#pragma endregion

  void HTMLLabelElement::SetActive(bool down, style::InvalidationScope invalidationScope) noexcept
  {
    if (down == Active())
    {
      return;
    }

    // Update our status first.
    HTMLElement::SetActive(down, invalidationScope);

    // Also update our corresponding control.
    if (auto element = Control())
    {
      element->SetActive(down);
    }
  }

  void HTMLLabelElement::SetHovered(bool over, style::InvalidationScope invalidationScope,
                                    HitTestRequest request) noexcept
  {
    if (over == Hovered())
    {
      return;
    }

    // Update our status first.
    HTMLElement::SetHovered(over, invalidationScope, request);

    // Also update our corresponding control.
    if (auto element = Control())
    {
      element->SetHovered(over);
    }
  }

  bool HTMLLabelElement::IsEventTargetedAtInteractiveDescendants(dom::Event &event) const noexcept
  {
    auto target = event.Target();
    auto *node = DynamicDowncast<dom::Node>(target.get());
    if (!node)
    {
      return false;
    }

    if (!dom::TreeQueries::IsShadowIncludingInclusiveAncestor(*this, *node))
    {
      return false;
    }

    for (auto *it = node; it && it != this; it = dom::TreeQueries::ParentOrShadowHostElement(*it))
    {
      auto *element = DynamicDowncast<HTMLElement>(*it);
      if (element && element->IsInteractiveContent())
      {
        return true;
      }
    }
    return false;
  }

  void HTMLLabelElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (dom::IsAnyClick(event) && !_processingClick)
    {
      auto control = Control();

      // If we can't find a control or if the control received the click event, then there's no need for us to
      // do anything.
      auto target = event.Target();
      auto *eventTarget = DynamicDowncast<dom::Node>(target.get());
      if (!control
          || (eventTarget && dom::TreeQueries::IsShadowIncludingInclusiveAncestor(*control, *eventTarget)))
      {
        HTMLElement::DefaultEventHandler(event);
        return;
      }

      // The activation behavior of a label element for events targeted at interactive content descendants of
      // a label element, and any descendants of those interactive content descendants, must be to do nothing.
      // https://html.spec.whatwg.org/#the-label-element
      if (IsEventTargetedAtInteractiveDescendants(event))
      {
        HTMLElement::DefaultEventHandler(event);
        return;
      }

      bool wasProcessingClick = std::exchange(_processingClick, true);

      (void)control->DispatchSimulatedClick(&event);

      (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets();
      if (FocusAlgorithms::IsFocusableArea(*control))
      {
        control->Focus({.Trigger = dom::FocusTrigger::Click});
      }

      event.SetDefaultHandled();
      _processingClick = wasProcessingClick;
    }

    HTMLElement::DefaultEventHandler(event);
  }
}