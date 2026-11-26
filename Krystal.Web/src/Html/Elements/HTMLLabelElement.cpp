#include "Krystal.Web/Html/Elements/HTMLLabelElement.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentFragment.hpp"
#include "Krystal.Web/Dom/Traversal/TreeTraversal.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include "Krystal.Web/Events/Pointer/MouseEvent.hpp"
#include "Krystal.Web/Html/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Web/Html/Elements/HTMLFormElement.hpp"
#include "Krystal.Web/Html/Forms/ValidatedFormListedElement.hpp"

namespace krys::web
{
  namespace
  {
    /// @brief @p element, resolved through its reference target, if that's a labelable element.
    KRYS_NODISCARD RefPtr<HTMLElement> ResolvedIfLabelable(Element &element) noexcept
    {
      auto resolved = element.ResolveReferenceTarget();
      auto *labelable = DynamicDowncast<HTMLElement>(resolved.get());
      return labelable && labelable->IsLabelable() ? ShareRefPtr(labelable) : nullptr;
    }

    /// @brief The resolved target of the ID @p id in @p label's tree, if that's a labelable element.
    KRYS_NODISCARD RefPtr<HTMLElement> ElementForAttributeIfLabelable(const HTMLLabelElement &label,
                                                                      DOMStringView id) noexcept
    {
      auto &root = dom::TreeQueries::Root(const_cast<HTMLLabelElement &>(label));
      RefPtr<Element> element;
      if (auto *document = DynamicDowncast<Document>(root))
      {
        element = document->GetElementById(id);
      }
      else if (auto *fragment = DynamicDowncast<DocumentFragment>(root))
      {
        element = fragment->GetElementById(id);
      }

      return element ? ResolvedIfLabelable(*element) : nullptr;
    }
  }

  HTMLLabelElement::HTMLLabelElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Label)
  {
  }

#pragma region HTMLLabelElement

  DOMString HTMLLabelElement::HtmlFor() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"for").value_or(u8"");
  }

  void HTMLLabelElement::HtmlFor(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"for", krys::move(value));
  }

  bool HTMLLabelElement::AccessKeyAction(bool sendMouseEvents) noexcept
  {
    if (auto control = Control())
    {
      return control->AccessKeyAction(sendMouseEvents);
    }
    return HTMLElement::AccessKeyAction(sendMouseEvents);
  }

  RefPtr<HTMLElement> HTMLLabelElement::Control() const noexcept
  {
    // 1. If label's for attribute is specified:
    if (auto forAttribute = GetAttribute(u8"for"))
    {
      // 1a. Let target be label's resolved for target element.
      // 1b. If target is not null and target is a labelable element, then return target.
      // 1c. Otherwise, return null.
      // (Only when connected.)
      return IsConnected() ? ElementForAttributeIfLabelable(*this, *forAttribute) : nullptr;
    }

    // 2. For each element descendant that is a descendant of label, in tree order:
    for (auto *node = dom::TreeQueries::Next(*this, this); node; node = dom::TreeQueries::Next(*node, this))
    {
      // 2a. Let candidate be the result of resolving the reference target on descendant.
      // 2b. If candidate is a labelable element, then return candidate.
      if (auto *descendant = DynamicDowncast<Element>(*node))
      {
        if (auto candidate = ResolvedIfLabelable(*descendant))
        {
          return candidate;
        }
      }
    }

    // 3. Return null.
    return nullptr;
  }

  RefPtr<HTMLElement> HTMLLabelElement::ControlForBindings() const noexcept
  {
    // 1. Let control be this's labeled control.
    // 2. If control is null, then return null.
    // 3. Return the result of retargeting control against this.
    // (A shadow host is an HTML element.)
    auto control = Control();
    return Downcast<HTMLElement>(RetargetForBindings(control.get()));
  }

  HTMLFormElement *HTMLLabelElement::Form() const noexcept
  {
    // 1. If the label element has no labeled control, then return null.
    // 2. If the label element's labeled control is not a form-associated element, then return null.
    // 3. Return the label element's labeled control's form owner (which can still be null).
    // (Every labelable form-associated element is a validated listed element.)
    if (auto element = Control())
    {
      if (auto *listedElement = element->AsValidatedFormListedElement())
      {
        return listedElement->Form();
      }
    }
    return nullptr;
  }

  RefPtr<Element> HTMLLabelElement::FormForBindings() const noexcept
  {
    // 1. Let control be this's labeled control.
    // 2. If control is null, then return null.
    // 3. If control is not a form-associated element, then return null.
    // 4. If control's form owner is null, then return null.
    // 5. Return the result of retargeting control's form owner against this.
    return RetargetForBindings(Form());
  }

#pragma endregion

  void HTMLLabelElement::SetActive(bool down, style::InvalidationScope invalidationScope) noexcept
  {
    if (down == Active())
    {
      return;
    }

    // The labeled control is active along with the label.
    HTMLElement::SetActive(down, invalidationScope);

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

    // The labeled control is hovered along with the label.
    HTMLElement::SetHovered(over, invalidationScope, request);

    if (auto element = Control())
    {
      element->SetHovered(over);
    }
  }

  bool HTMLLabelElement::IsEventTargetedAtInteractiveDescendants(Event &event) const noexcept
  {
    auto target = event.Target();
    auto *node = DynamicDowncast<Node>(target.get());
    if (!node)
    {
      return false;
    }

    if (!dom::TreeQueries::IsShadowIncludingInclusiveAncestor(*this, *node))
    {
      return false;
    }

    for (auto *it = node; it && it != this; it = dom::TreeTraversal::ParentOrShadowHostElement(*it))
    {
      auto *element = DynamicDowncast<HTMLElement>(*it);
      if (element && element->IsInteractiveContent())
      {
        return true;
      }
    }
    return false;
  }

  void HTMLLabelElement::DefaultEventHandler(Event &event) noexcept
  {
    if (pointer_events::IsAnyClick(event) && !_processingClick)
    {
      auto control = Control();

      // If we can't find a control or if the control received the click event, then there's no need for us to
      // do anything.
      auto target = event.Target();
      auto *eventTarget = DynamicDowncast<Node>(target.get());
      if (!control
          || (eventTarget && dom::TreeQueries::IsShadowIncludingInclusiveAncestor(*control, *eventTarget)))
      {
        HTMLElement::DefaultEventHandler(event);
        return;
      }

      // The activation behavior of a label element for events targeted at interactive content descendants of
      // a label element, and any descendants of those interactive content descendants, must be to do nothing.
      // https://html.spec.whatwg.org/multipage/forms.html#the-label-element
      if (IsEventTargetedAtInteractiveDescendants(event))
      {
        HTMLElement::DefaultEventHandler(event);
        return;
      }

      bool wasProcessingClick = std::exchange(_processingClick, true);

      (void)control->DispatchSimulatedClick(&event);

      (void)NodeDocument().UpdateLayoutIgnorePendingStylesheets();
      if (html::FocusAlgorithms::IsFocusableArea(*control))
      {
        control->Focus({.Trigger = dom::FocusTrigger::Click});
      }

      event.SetDefaultHandled();
      _processingClick = wasProcessingClick;
    }

    HTMLElement::DefaultEventHandler(event);
  }
}
