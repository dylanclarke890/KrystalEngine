/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2022 Apple Inc. All rights reserved.
 *           (C) 2006 Alexey Proskuryakov (ap@nypop.com)
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

#include "Krystal.Booey/HTML/Elements/HTMLFormControlElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/ScopedEventQueue.hpp"
#include "Krystal.Booey/DOM/Internals/PopoverData.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Booey/Style/TreeResolver.hpp"

namespace krys::boo
{
  template <>
  struct html::EnumeratedAttributeTraits<"popovertargetaction", HTMLFormControlElement>
  {
    using State = html::PopoverTargetActionAttributeState;
    static inline const html::KeywordStateMapping<State> StateMappings[] = {
      {u8"toggle", State::Toggle}, {u8"show", State::Show}, {u8"hide", State::Hide}};

    constexpr static State MissingValueDefault = State::Toggle;
    constexpr static State InvalidValueDefault = State::Toggle;
  };

  HTMLFormControlElement::HTMLFormControlElement(Document &document, html::HTMLElementInterface interface,
                                                 html::HTMLElementFlags flags) noexcept
      : HTMLElement(document, interface, flags, dom::NodeFlag::HasCustomStyleResolveCallbacks)
  {
  }

  void HTMLFormControlElement::DidAttachRenderers() noexcept
  {
    // The call to UpdateFromElement() needs to go after the call through
    // to the base class's attach() because that can sometimes do a close
    // on the renderer.
    if (auto *renderer = Renderer())
    {
      renderer->UpdateFromElement();
    }
  }

  void HTMLFormControlElement::DidRecalcStyle(EnumSet<style::Change>) noexcept
  {
    // UpdateFromElement() can cause the selection to change, and in turn
    // trigger synchronous layout, so it must not be called during style recalc.
    if (Renderer())
    {
      style::DeprecatedQueuePostResolutionCallback(
        [element = ShareRef(*this)]
        {
          if (auto *renderer = element->Renderer())
          {
            renderer->UpdateFromElement();
          }
        });
    }
  }

  HTMLFormControlElement::~HTMLFormControlElement() noexcept
  {
    ClearForm();
  }

  void HTMLFormControlElement::DispatchBlurEvent(EventTarget *relatedTarget) noexcept
  {
    HTMLElement::DispatchBlurEvent(relatedTarget);
    HideVisibleValidationMessage();
  }

  bool HTMLFormControlElement::FormNoValidate() const noexcept
  {
    return HasAttribute(u8"formnovalidate");
  }

  void HTMLFormControlElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    if (auto *parent = ParentNode())
    {
      html::ValidatedFormListedElement::InsertedIntoAncestor(IsConnected(), *parent);
    }
  }

  void HTMLFormControlElement::OnPostConnection() noexcept
  {
    HTMLElement::OnPostConnection();
    html::ValidatedFormListedElement::DidFinishInsertingNode();
  }

  void HTMLFormControlElement::OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    html::ValidatedFormListedElement::RemovedFromAncestor(oldAncestor.IsConnected(), oldAncestor);
  }

  void HTMLFormControlElement::OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);

    // A move is a removal and an insertion that keep the element connected.
    html::ValidatedFormListedElement::RemovedFromAncestor(false, oldAncestor);
    if (auto *parent = ParentNode())
    {
      html::ValidatedFormListedElement::InsertedIntoAncestor(IsConnected(), *parent);
    }
    if (IsConnected())
    {
      html::ValidatedFormListedElement::DidFinishInsertingNode();
    }
  }

  void HTMLFormControlElement::OnAdopt(Document &oldDocument) noexcept
  {
    HTMLElement::OnAdopt(oldDocument);
    html::ValidatedFormListedElement::DidMoveToNewDocument();
  }

  void HTMLFormControlElement::OnAttributeChanged(const DOMStringAtom &localName,
                                                  Maybe<DOMStringView> oldValue, Maybe<DOMStringView> value,
                                                  const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != null)
    {
      return;
    }

    if (localName == u8"required")
    {
      bool newRequired = value.has_value();
      if (_isRequired != newRequired)
      {
        style::PseudoClassChangeInvalidation requiredInvalidation(
          *this, {
                   {css::SelectorPseudoClass::Required, newRequired},
                   {css::SelectorPseudoClass::Optional, !newRequired},
                 });
        _isRequired = newRequired;
        RequiredStateChanged();
      }
    }
    else
    {
      html::ValidatedFormListedElement::ParseAttribute(localName, value);
    }
  }

  void HTMLFormControlElement::DisabledStateChanged() noexcept
  {
    html::ValidatedFormListedElement::DisabledStateChanged();
    if (auto *renderer = Renderer(); renderer && renderer->Style().HasUsedAppearance())
    {
      renderer->Repaint();
    }
  }

  void HTMLFormControlElement::RequiredStateChanged() noexcept
  {
    UpdateValidity();
  }

  void HTMLFormControlElement::SetChangedSinceLastFormControlChangeEvent(bool changed) noexcept
  {
    _wasChangedSinceLastFormControlChangeEvent = changed;
  }

  void HTMLFormControlElement::DispatchChangeEvent() noexcept
  {
    dom::ScopedEventQueue::DispatchOrQueue(Event::Create(event_names::Change, true, false), ShareRef(*this));
  }

  void HTMLFormControlElement::DispatchCancelEvent() noexcept
  {
    auto event = Event::Create(event_names::Cancel, true, false);
    (void)dom::EventDispatcher::DispatchToTarget(*event, this);
  }

  void HTMLFormControlElement::DispatchFormControlChangeEvent() noexcept
  {
    DispatchChangeEvent();
    SetChangedSinceLastFormControlChangeEvent(false);
    SetInteractedWithSinceLastFormSubmitEvent(true);
  }

  void HTMLFormControlElement::DispatchFormControlInputEvent() noexcept
  {
    SetChangedSinceLastFormControlChangeEvent(true);
    dom::ScopedEventQueue::DispatchOrQueue(Event::Create(event_names::Input, true, false, true),
                                           ShareRef(*this));
  }

  Maybe<DOMString> HTMLFormControlElement::ResultForDialogSubmit() const noexcept
  {
    return GetAttribute(u8"value");
  }

  // https://html.spec.whatwg.org/#popover-target-element
  RefPtr<HTMLElement> HTMLFormControlElement::PopoverTargetElement() const noexcept
  {
    // 1. If node is not a button, then return null.
    if (auto *inputElement = DynamicDowncast<HTMLInputElement>(*this))
    {
      if (!inputElement->IsTextButton() && !inputElement->IsImageButton())
      {
        return nullptr;
      }
    }
    else if (!Is<HTMLButtonElement>(*this))
    {
      return nullptr;
    }

    // 2. If node is disabled, then return null.
    if (IsDisabledFormControl())
    {
      return nullptr;
    }

    // 3. If node has a form owner and node is a submit button, then return null.
    if (Form() && IsSubmitButton())
    {
      return nullptr;
    }

    // 4. Let popoverElement be node's resolved popovertarget target element.
    // 5. If popoverElement is null, then return null.
    // 6. If popoverElement's popover attribute is in the No Popover state, then return null.
    // 7. Return popoverElement.
    RefPtr<boo::Element> element = ResolvedElementForAttribute(u8"popovertarget");
    auto *popover = element ? DynamicDowncast<HTMLElement>(*element) : nullptr;
    if (popover && popover->GetPopoverState() != dom::PopoverState::None)
    {
      return ShareRefPtr(popover);
    }
    return nullptr;
  }

  RefPtr<boo::Element> HTMLFormControlElement::PopoverTargetElementForBindings() const noexcept
  {
    // The getter steps are to return the result of running this's get the unresolved popovertarget target
    // element (a host, not what its reference target names).
    return ElementForAttributeInternal(u8"popovertarget");
  }

  void HTMLFormControlElement::PopoverTargetElementForBindings(boo::Element *element) noexcept
  {
    html::CEReactionsScope ceReactions;
    SetElementAttribute(u8"popovertarget", element);
  }

  // https://html.spec.whatwg.org/#dom-popovertargetaction
  // The popoverTargetAction IDL attribute must reflect the popovertargetaction attribute, limited to only
  // known values.
  DOMString HTMLFormControlElement::PopoverTargetAction() const noexcept
  {
    return html::ReflectEnumerated<"popovertargetaction", HTMLFormControlElement>(*this);
  }

  void HTMLFormControlElement::PopoverTargetAction(DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    html::Reflect<DOMString>(*this, u8"popovertargetaction", krys::move(value));
  }

  // https://html.spec.whatwg.org/#popover-target-attribute-activation-behavior
  void HTMLFormControlElement::HandlePopoverTargetAction(EventTarget *eventTarget) noexcept
  {
    // 1. Let popover be node's popover target element.
    // 2. If popover is null, then return.
    RefPtr<HTMLElement> popover = PopoverTargetElement();
    if (!popover)
    {
      return;
    }
    krys_debug_assert(popover->GetPopoverData());

    // 3. If eventTarget is a shadow-including inclusive descendant of popover and popover is a
    //    shadow-including descendant of node, then return.
    if (auto *eventTargetNode = DynamicDowncast<Node>(eventTarget);
        eventTargetNode && dom::TreeQueries::IsShadowIncludingInclusiveDescendant(*eventTargetNode, *popover)
        && dom::TreeQueries::IsShadowIncludingDescendant(*popover, *this))
    {
      return;
    }

    auto action = html::EnumeratedAttribute<"popovertargetaction", HTMLFormControlElement>::ResolveState(
      GetAttribute(u8"popovertargetaction"));
    // 4. If node's popovertargetaction attribute is in the show state and popover's popover visibility state
    //    is showing, then return.
    if (action == html::PopoverTargetActionAttributeState::Show && popover->IsPopoverShowing())
    {
      return;
    }
    // 5. If node's popovertargetaction attribute is in the hide state and popover's popover visibility state
    //    is hidden, then return.
    if (action == html::PopoverTargetActionAttributeState::Hide && !popover->IsPopoverShowing())
    {
      return;
    }

    // 6. If popover's popover visibility state is showing, then run the hide popover algorithm given popover,
    //    true, true, false, and node.
    if (popover->IsPopoverShowing())
    {
      (void)popover->HidePopoverInternal(dom::FocusPreviousElement::Yes, dom::FireEvents::Yes, false, this);
    }
    // 7. Otherwise:
    // 7a. Let validity be the result of running check popover validity given popover, false, and null. If
    // this
    //     throws an exception, catch it, and let validity be false.
    // 7b. If validity is true, then run show popover given popover, false, and node.
    // (Show popover checks the validity first, and doesn't throw.)
    else
    {
      (void)popover->ShowPopoverInternal(this, false);
    }
  }
}
