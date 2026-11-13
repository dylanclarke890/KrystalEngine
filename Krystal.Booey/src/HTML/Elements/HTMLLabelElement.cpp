/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 *           (C) 2006 Alexey Proskuryakov (ap@nypop.com)
 * Copyright (C) 2014 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Booey/HTML/Elements/HTMLLabelElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Forms/ValidatedFormListedElement.hpp"

namespace krys::boo::html
{
  namespace
  {
    /// @brief @p element, resolved through its reference target, if that's a labelable element.
    KRYS_NODISCARD RefPtr<HTMLElement> ResolvedIfLabelable(dom::Element &element) noexcept
    {
      auto resolved = element.ResolveReferenceTarget();
      auto *labelable = DynamicDowncast<HTMLElement>(resolved.get());
      return labelable && labelable->IsLabelable() ? ShareRefPtr(labelable) : nullptr;
    }

    /// @brief The resolved target of the ID @p id in @p label's tree, if that's a labelable element.
    KRYS_NODISCARD RefPtr<HTMLElement> ElementForAttributeIfLabelable(const HTMLLabelElement &label,
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

      return element ? ResolvedIfLabelable(*element) : nullptr;
    }
  }

  HTMLLabelElement::HTMLLabelElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Label)
  {
  }

#pragma region HTMLLabelElement

  dom::DOMString HTMLLabelElement::HtmlFor() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"for").value_or(u8"");
  }

  void HTMLLabelElement::HtmlFor(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"for", krys::move(value));
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
      if (auto *descendant = DynamicDowncast<dom::Element>(*node))
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

  RefPtr<dom::Element> HTMLLabelElement::FormForBindings() const noexcept
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