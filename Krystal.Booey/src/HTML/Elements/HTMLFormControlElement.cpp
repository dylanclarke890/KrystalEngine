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

namespace krys::boo::html
{
  template <>
  struct EnumeratedAttributeTraits<"popovertargetaction", HTMLFormControlElement>
  {
    using State = PopoverTargetActionAttributeState;
    static inline const KeywordStateMapping<State> StateMappings[] = {
      {u8"toggle", State::Toggle}, {u8"show", State::Show}, {u8"hide", State::Hide}};

    constexpr static State MissingValueDefault = State::Toggle;
    constexpr static State InvalidValueDefault = State::Toggle;
  };

  HTMLFormControlElement::HTMLFormControlElement(dom::Document &document, HTMLElementInterface interface,
                                                 HTMLElementFlags flags) noexcept
      : HTMLElement(document, interface, flags, dom::NodeFlags::HasCustomStyleResolveCallbacks)
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

  void HTMLFormControlElement::DispatchBlurEvent(dom::EventTarget *relatedTarget) noexcept
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
      ValidatedFormListedElement::InsertedIntoAncestor(IsConnected(), *parent);
    }
  }

  void HTMLFormControlElement::OnPostConnection() noexcept
  {
    HTMLElement::OnPostConnection();
    ValidatedFormListedElement::DidFinishInsertingNode();
  }

  void HTMLFormControlElement::OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnRemove(isSubtreeRoot, oldAncestor);
    ValidatedFormListedElement::RemovedFromAncestor(oldAncestor.IsConnected(), oldAncestor);
  }

  void HTMLFormControlElement::OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept
  {
    HTMLElement::OnMove(isSubtreeRoot, oldAncestor);

    // A move is a removal and an insertion that keep the element connected.
    ValidatedFormListedElement::RemovedFromAncestor(false, oldAncestor);
    if (auto *parent = ParentNode())
    {
      ValidatedFormListedElement::InsertedIntoAncestor(IsConnected(), *parent);
    }
    if (IsConnected())
    {
      ValidatedFormListedElement::DidFinishInsertingNode();
    }
  }

  void HTMLFormControlElement::OnAdopt(dom::Document &oldDocument) noexcept
  {
    HTMLElement::OnAdopt(oldDocument);
    ValidatedFormListedElement::DidMoveToNewDocument();
  }

  void HTMLFormControlElement::OnAttributeChanged(dom::DOMStringAtom localName,
                                                  Maybe<dom::DOMStringView> oldValue,
                                                  Maybe<dom::DOMStringView> value,
                                                  dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI != dom::DOMStringAtom::Null())
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
      ValidatedFormListedElement::ParseAttribute(localName, value);
    }
  }

  void HTMLFormControlElement::DisabledStateChanged() noexcept
  {
    ValidatedFormListedElement::DisabledStateChanged();
    if (auto *renderer = Renderer(); renderer && renderer->Style().HasUsedAppearance())
    {
      renderer->Repaint();
    }
  }

  void HTMLFormControlElement::ReadOnlyStateChanged() noexcept
  {
    ValidatedFormListedElement::ReadOnlyStateChanged();

    // Some input pseudo-classes, like :in-range and :out-of-range, depend on the readonly state.
    // FIXME: Use PseudoClassChangeInvalidation instead for :has() support and more efficiency.
    InvalidateStyleForSubtree();
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
    dom::ScopedEventQueue::DispatchOrQueue(
      dom::Event::Create(dom::DOMStringAtom(EventNames::Change), true, false), ShareRef(*this));
  }

  void HTMLFormControlElement::DispatchCancelEvent() noexcept
  {
    auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Cancel), true, false);
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
    dom::ScopedEventQueue::DispatchOrQueue(
      dom::Event::Create(dom::DOMStringAtom(EventNames::Input), true, false, true), ShareRef(*this));
  }

  Maybe<dom::DOMString> HTMLFormControlElement::ResultForDialogSubmit() const noexcept
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
    RefPtr<dom::Element> element = ResolvedElementForAttribute(dom::DOMStringAtom(u8"popovertarget"));
    auto *popover = element ? DynamicDowncast<HTMLElement>(*element) : nullptr;
    if (popover && popover->GetPopoverState() != dom::PopoverState::None)
    {
      return ShareRefPtr(popover);
    }
    return nullptr;
  }

  RefPtr<dom::Element> HTMLFormControlElement::PopoverTargetElementForBindings() const noexcept
  {
    // The getter steps are to return the result of running this's get the unresolved popovertarget target
    // element (a host, not what its reference target names).
    return ElementForAttributeInternal(dom::DOMStringAtom(u8"popovertarget"));
  }

  void HTMLFormControlElement::PopoverTargetElementForBindings(dom::Element *element) noexcept
  {
    CEReactionsScope ceReactions;
    SetElementAttribute(dom::DOMStringAtom(u8"popovertarget"), element);
  }

  // https://html.spec.whatwg.org/#dom-popovertargetaction
  // The popoverTargetAction IDL attribute must reflect the popovertargetaction attribute, limited to only
  // known values.
  dom::DOMString HTMLFormControlElement::PopoverTargetAction() const noexcept
  {
    return ReflectEnumerated<"popovertargetaction", HTMLFormControlElement>(*this);
  }

  void HTMLFormControlElement::PopoverTargetAction(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    Reflect<dom::DOMString>(*this, u8"popovertargetaction", krys::move(value));
  }

  // https://html.spec.whatwg.org/#popover-target-attribute-activation-behavior
  void HTMLFormControlElement::HandlePopoverTargetAction(dom::EventTarget *eventTarget) noexcept
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
    if (auto *eventTargetNode = DynamicDowncast<dom::Node>(eventTarget);
        eventTargetNode && dom::TreeQueries::IsShadowIncludingInclusiveDescendant(*eventTargetNode, *popover)
        && dom::TreeQueries::IsShadowIncludingDescendant(*popover, *this))
    {
      return;
    }

    auto action = EnumeratedAttribute<"popovertargetaction", HTMLFormControlElement>::ResolveState(
      GetAttribute(dom::DOMStringAtom(u8"popovertargetaction")));
    // 4. If node's popovertargetaction attribute is in the show state and popover's popover visibility state
    //    is showing, then return.
    if (action == PopoverTargetActionAttributeState::Show && popover->IsPopoverShowing())
    {
      return;
    }
    // 5. If node's popovertargetaction attribute is in the hide state and popover's popover visibility state
    //    is hidden, then return.
    if (action == PopoverTargetActionAttributeState::Hide && !popover->IsPopoverShowing())
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
