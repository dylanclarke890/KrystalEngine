/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *           (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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
 */

#include "Krystal.Booey/HTML/Elements/HTMLAnchorElement.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Algorithms/HyperlinkAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/Page/Settings.hpp"

#include "Krystal.Core/Strings/ASCII.hpp"

namespace krys::boo
{
  HTMLAnchorElement::HTMLAnchorElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Anchor)
  {
    SetEventTargetFlag(dom::EventTargetFlags::HasActivationBehavior);
  }

  namespace
  {
    KRYS_NODISCARD bool IsShiftKeyDown(const Event &event) noexcept
    {
      const auto *mouseEvent = DynamicDowncast<MouseEvent>(event);
      return mouseEvent && mouseEvent->ShiftKey();
    }
  }

  void HTMLAnchorElement::ActivationBehavior(Event &event) noexcept
  {
    if (!TreatLinkAsLive(IsShiftKeyDown(event)))
    {
      return;
    }
    html::HyperlinkAlgorithms::ActivationBehavior(*this, event);
  }

  bool HTMLAnchorElement::TreatLinkAsLive(bool shiftKey) const noexcept
  {
    return !HasEditableStyle()
           || IsEditableLinkLive(NodeDocument().GetSettings().EditableLinkBehavior, shiftKey);
  }

  bool HTMLAnchorElement::IsLiveLink() const noexcept
  {
    return css::IsLink(*this) && TreatLinkAsLive(_wasShiftKeyDownOnMouseDown);
  }

  void HTMLAnchorElement::DefaultEventHandler(Event &event) noexcept
  {
    if (css::IsLink(*this))
    {
      // The shift key at the press is kept for the drag it may start; a drag comes after the mouse leaves, so
      // it's cleared when the mouse comes over the link again.
      if (event.Type() == event_names::MouseDown)
      {
        _wasShiftKeyDownOnMouseDown = IsShiftKeyDown(event);
      }
      else if (event.Type() == event_names::MouseOver)
      {
        _wasShiftKeyDownOnMouseDown = false;
      }

      if (Focused() && html::HyperlinkAlgorithms::IsEnterKeyKeydownEvent(event) && TreatLinkAsLive(false))
      {
        event.SetDefaultHandled();
        (void)DispatchSimulatedClick(&event);
        return;
      }

      // A click is followed as the activation behaviour; other buttons' clicks (the middle button opening the
      // link elsewhere) are followed here, and the host decides from the button.
      if (event.Type() == event_names::AuxClick && MouseEvent::CanTriggerActivationBehavior(event)
          && TreatLinkAsLive(IsShiftKeyDown(event)))
      {
        html::HyperlinkAlgorithms::FollowTheHyperlink(*this, event);
        return;
      }
    }

    HTMLElement::DefaultEventHandler(event);
  }

  bool HTMLAnchorElement::IsInteractiveContent() const noexcept
  {
    return css::IsLink(*this);
  }

  bool HTMLAnchorElement::CanStartSelection() const noexcept
  {
    if (!css::IsLink(*this))
    {
      return HTMLElement::CanStartSelection();
    }
    return HasEditableStyle();
  }

#pragma region HTMLAnchorElement

  DOMString HTMLAnchorElement::Target() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLAnchorElement::Target(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"target", krys::move(value));
  }

  DOMString HTMLAnchorElement::Download() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"download").value_or(u8"");
  }

  void HTMLAnchorElement::Download(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"download", krys::move(value));
  }

  dom::USVString HTMLAnchorElement::Ping() const noexcept
  {
    return html::Reflect<dom::USVString>(*this, u8"ping");
  }

  void HTMLAnchorElement::Ping(dom::USVString value) noexcept
  {
    return html::Reflect<dom::USVString>(*this, u8"ping", krys::move(value));
  }

  DOMString HTMLAnchorElement::Rel() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLAnchorElement::Rel(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"rel", krys::move(value));
  }

  dom::DOMTokenList &HTMLAnchorElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList =
        dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"rel",
                                                   [](Document &, DOMStringView token)
                                                   {
                                                     return text::IsASCIICaselessEqual(token, u8"noreferrer")
                                                            || text::IsASCIICaselessEqual(token, u8"noopener")
                                                            || text::IsASCIICaselessEqual(token, u8"opener");
                                                   });
    }

    return *_relList;
  }

  void HTMLAnchorElement::OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                             Maybe<DOMStringView> value,
                                             const DOMStringAtom &namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI == null && localName == u8"rel" && _relList)
    {
      _relList->AssociatedAttributeValueChanged();
    }
  }

  DOMString HTMLAnchorElement::HrefLang() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"hreflang").value_or(u8"");
  }

  void HTMLAnchorElement::HrefLang(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"hreflang", krys::move(value));
  }

  DOMString HTMLAnchorElement::Type() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLAnchorElement::Type(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"type", krys::move(value));
  }

  DOMString HTMLAnchorElement::Text() const noexcept
  {
    return dom::TextAlgorithms::DescendantTextContent(*this);
  }

  ExceptionOr<void> HTMLAnchorElement::Text(DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

  DOMString HTMLAnchorElement::ReferrerPolicy() const noexcept
  {
    return html::ReflectEnumerated<"referrerpolicy">(*this);
  }

  void HTMLAnchorElement::ReferrerPolicy(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

#pragma endregion

#pragma region HTMLHyperlinkElementUtils

  dom::USVString HTMLAnchorElement::Href() const noexcept
  {
    return html::ReflectURL<dom::USVString>(*this, u8"href");
  }

  void HTMLAnchorElement::Href(dom::USVString value) noexcept
  {
    html::ReflectURL<dom::USVString>(*this, u8"href", krys::move(value));
  }

  dom::USVString HTMLAnchorElement::ToString() const noexcept
  {
    return Href();
  }

  net::URL HTMLAnchorElement::FullURL() const noexcept
  {
    return html::HyperlinkAlgorithms::Href(*this);
  }

  void HTMLAnchorElement::SetFullURL(const net::URL &url) noexcept
  {
    html::CEReactionsScope ceReactions;
    html::HyperlinkAlgorithms::SetHref(*this, url);
  }

#pragma endregion

#pragma region HTMLAnchorElement Obsolete members

  DOMString HTMLAnchorElement::Coords() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"coords").value_or(u8"");
  }

  void HTMLAnchorElement::Coords(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"coords", krys::move(value));
  }

  DOMString HTMLAnchorElement::Charset() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLAnchorElement::Charset(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"charset", krys::move(value));
  }

  DOMString HTMLAnchorElement::Name() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLAnchorElement::Name(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"name", krys::move(value));
  }

  DOMString HTMLAnchorElement::Rev() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"rev").value_or(u8"");
  }

  void HTMLAnchorElement::Rev(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"rev", krys::move(value));
  }

  DOMString HTMLAnchorElement::Shape() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"shape").value_or(u8"");
  }

  void HTMLAnchorElement::Shape(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"shape", krys::move(value));
  }

#pragma endregion
}
