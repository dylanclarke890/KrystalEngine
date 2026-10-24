#include "Krystal.Booey/HTML/HTMLMetaElement.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::html
{
#pragma region HTMLMetaElement

  HTMLMetaElement::HTMLMetaElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Meta)
  {
  }

  dom::DOMString HTMLMetaElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLMetaElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLMetaElement::HttpEquiv() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"http-equiv").value_or(u8"");
  }

  void HTMLMetaElement::HttpEquiv(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"http-equiv", krys::move(value));
  }

  dom::DOMString HTMLMetaElement::Content() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"content").value_or(u8"");
  }

  void HTMLMetaElement::Content(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"content", krys::move(value));
  }

  dom::DOMString HTMLMetaElement::Media() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLMetaElement::Media(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"media", krys::move(value));
  }

#pragma endregion

#pragma region HTMLMetaElement Obsolete members

  dom::DOMString HTMLMetaElement::Scheme() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"scheme").value_or(u8"");
  }

  void HTMLMetaElement::Scheme(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"scheme", krys::move(value));
  }

#pragma endregion

  void HTMLMetaElement::OnInsert() noexcept
  {
    HTMLElement::OnInsert();
    Process();
  }

  void HTMLMetaElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                           Maybe<dom::DOMStringView> value,
                                           dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if ((localName == u8"http-equiv" || localName == u8"content")
        && namespaceURI == dom::DOMStringAtom::Null())
    {
      Process();
    }
  }

  void HTMLMetaElement::Process() noexcept
  {
    // When a meta element is inserted into the document, if its http-equiv attribute is present and
    // represents one of the pragma directives, run its algorithm.
    if (!IsConnected())
    {
      return;
    }
    auto httpEquiv = GetAttribute(u8"http-equiv");
    auto content = GetAttribute(u8"content");
    if (!httpEquiv || !content)
    {
      return;
    }
    NodeDocument().ProcessMetaHttpEquiv(*httpEquiv, *content);
  }
}
