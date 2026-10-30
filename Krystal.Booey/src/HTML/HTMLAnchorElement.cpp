#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/HTML/Algorithms/HyperlinkAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"

namespace krys::boo::html
{
  HTMLAnchorElement::HTMLAnchorElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Anchor)
  {
    SetEventTargetFlag(dom::EventTargetFlags::HasActivationBehavior);
  }

  void HTMLAnchorElement::ActivationBehavior(dom::Event &event) noexcept
  {
    // If element has no href attribute, then return.
    if (!css::IsLink(*this))
    {
      return;
    }
    // FIXME: Links in editable content follow the editable link behaviour setting (Settings::
    // editableLinkBehavior), once there is one; by default they're live, as here.
    HyperlinkAlgorithms::FollowTheHyperlink(*this, event);
  }

  void HTMLAnchorElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    if (css::IsLink(*this))
    {
      if (Focused() && HyperlinkAlgorithms::IsEnterKeyKeydownEvent(event))
      {
        event.SetDefaultHandled();
        (void)DispatchSimulatedClick(&event);
        return;
      }

      // A click is followed as the activation behaviour; other buttons' clicks (the middle button opening the
      // link elsewhere) are followed here, and the host decides from the button.
      if (event.Type() == EventNames::AuxClick && dom::MouseEvent::CanTriggerActivationBehavior(event))
      {
        HyperlinkAlgorithms::FollowTheHyperlink(*this, event);
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

  dom::DOMString HTMLAnchorElement::Target() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLAnchorElement::Target(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"target", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Download() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"download").value_or(u8"");
  }

  void HTMLAnchorElement::Download(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"download", krys::move(value));
  }

  dom::USVString HTMLAnchorElement::Ping() const noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"ping");
  }

  void HTMLAnchorElement::Ping(dom::USVString &&value) noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"ping", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Rel() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLAnchorElement::Rel(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"rel", krys::move(value));
  }

  dom::DOMTokenList &HTMLAnchorElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"rel");
    }

    return *_relList;
  }

  void HTMLAnchorElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI == dom::DOMStringAtom::Null() && localName == u8"rel" && _relList)
    {
      _relList->AssociatedAttributeValueChanged();
    }
  }

  dom::DOMString HTMLAnchorElement::HrefLang() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"hreflang").value_or(u8"");
  }

  void HTMLAnchorElement::HrefLang(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"hreflang", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLAnchorElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Text() const noexcept
  {
    return dom::TextAlgorithms::DescendantTextContent(*this);
  }

  dom::ExceptionOr<void> HTMLAnchorElement::Text(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    return dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

  dom::DOMString HTMLAnchorElement::ReferrerPolicy() const noexcept
  {
    // FIXME: ReferrerPolicy should be limited to known values
    return Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLAnchorElement::ReferrerPolicy(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

#pragma endregion

#pragma region HTMLHyperlinkElementUtils

  dom::USVString HTMLAnchorElement::Href() const noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"href");
  }

  void HTMLAnchorElement::Href(dom::USVString &&value) noexcept
  {
    Reflection::ReflectURL<dom::USVString>(*this, u8"href", krys::move(value));
  }

  dom::USVString HTMLAnchorElement::ToString() const noexcept
  {
    return Href();
  }

  void HTMLAnchorElement::Protocol(dom::DOMStringView value) noexcept
  {
    if (!FullURL().IsValid())
    {
      return;
    }
    URLDecomposition::Protocol(value);
  }

  url::URL HTMLAnchorElement::FullURL() const noexcept
  {
    return HyperlinkAlgorithms::Href(*this);
  }

  void HTMLAnchorElement::SetFullURL(const url::URL &url) noexcept
  {
    CEReactionsScope ceReactions;
    HyperlinkAlgorithms::SetHref(*this, url);
  }

#pragma endregion

#pragma region HTMLAnchorElement Obsolete members

  dom::DOMString HTMLAnchorElement::Coords() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"coords").value_or(u8"");
  }

  void HTMLAnchorElement::Coords(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"coords", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Charset() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLAnchorElement::Charset(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"charset", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLAnchorElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Rev() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"rev").value_or(u8"");
  }

  void HTMLAnchorElement::Rev(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"rev", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Shape() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"shape").value_or(u8"");
  }

  void HTMLAnchorElement::Shape(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"shape", krys::move(value));
  }

#pragma endregion
}