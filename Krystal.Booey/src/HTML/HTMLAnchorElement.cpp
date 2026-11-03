#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Algorithms/HyperlinkAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"

#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  HTMLAnchorElement::HTMLAnchorElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Anchor)
  {
    SetEventTargetFlag(dom::EventTargetFlags::HasActivationBehavior);
  }

  void HTMLAnchorElement::ActivationBehavior(dom::Event &event) noexcept
  {
    HyperlinkAlgorithms::ActivationBehavior(*this, event);
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
    return Reflect<dom::DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLAnchorElement::Target(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"target", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Download() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"download").value_or(u8"");
  }

  void HTMLAnchorElement::Download(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"download", krys::move(value));
  }

  dom::USVString HTMLAnchorElement::Ping() const noexcept
  {
    return Reflect<dom::USVString>(*this, u8"ping");
  }

  void HTMLAnchorElement::Ping(dom::USVString value) noexcept
  {
    return Reflect<dom::USVString>(*this, u8"ping", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Rel() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLAnchorElement::Rel(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"rel", krys::move(value));
  }

  dom::DOMTokenList &HTMLAnchorElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = dom::ElementAlgorithms::CreateDOMTokenList(
        *this, u8"rel",
        [](dom::Document &, dom::DOMStringView token)
        {
          return text::IsASCIICaselessEqual(token, utf8_stringview(u8"noreferrer"))
                 || text::IsASCIICaselessEqual(token, utf8_stringview(u8"noopener"))
                 || text::IsASCIICaselessEqual(token, utf8_stringview(u8"opener"));
        });
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
    return Reflect<dom::DOMString>(*this, u8"hreflang").value_or(u8"");
  }

  void HTMLAnchorElement::HrefLang(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"hreflang", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Type() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLAnchorElement::Type(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Text() const noexcept
  {
    return dom::TextAlgorithms::DescendantTextContent(*this);
  }

  dom::ExceptionOr<void> HTMLAnchorElement::Text(dom::DOMString value) noexcept
  {
    CEReactionsScope ceReactions;
    return dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

  dom::DOMString HTMLAnchorElement::ReferrerPolicy() const noexcept
  {
    return EnumeratedAttribute<"referrerpolicy", HTMLElement>::ResolveCanonicalKeyword<dom::DOMString>(
      Reflect<dom::DOMString>(*this, u8"referrerpolicy"));
  }

  void HTMLAnchorElement::ReferrerPolicy(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

#pragma endregion

#pragma region HTMLHyperlinkElementUtils

  dom::USVString HTMLAnchorElement::Href() const noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"href");
  }

  void HTMLAnchorElement::Href(dom::USVString value) noexcept
  {
    ReflectURL<dom::USVString>(*this, u8"href", krys::move(value));
  }

  dom::USVString HTMLAnchorElement::ToString() const noexcept
  {
    return Href();
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
    return Reflect<dom::DOMString>(*this, u8"coords").value_or(u8"");
  }

  void HTMLAnchorElement::Coords(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"coords", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Charset() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLAnchorElement::Charset(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"charset", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Name() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLAnchorElement::Name(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Rev() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"rev").value_or(u8"");
  }

  void HTMLAnchorElement::Rev(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"rev", krys::move(value));
  }

  dom::DOMString HTMLAnchorElement::Shape() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"shape").value_or(u8"");
  }

  void HTMLAnchorElement::Shape(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"shape", krys::move(value));
  }

#pragma endregion
}