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
    // TODO(HTMLMETAELEMENT, HTML)" 'http-equiv' is an enumerated attribute, don't think it's limited to known
    // values though.
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
}
