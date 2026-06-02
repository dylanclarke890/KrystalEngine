#include "Krystal.HTML/HTML/HTMLMetaElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
#pragma region HTMLMetaElement

  HTMLMetaElement::HTMLMetaElement(Document &document) noexcept : HTMLElement(document, HTMLTagName::Meta)
  {
  }

  DOMString HTMLMetaElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLMetaElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

  DOMString HTMLMetaElement::HttpEquiv() const noexcept
  {
    // TODO(HTMLMETAELEMENT, HTML)" 'http-equiv' is an enumerated attribute, don't think it's limited to known
    // values though.
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"http-equiv").value_or(u8"");
  }

  void HTMLMetaElement::HttpEquiv(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"http-equiv", Krys::Move(value));
  }

  DOMString HTMLMetaElement::Content() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"content").value_or(u8"");
  }

  void HTMLMetaElement::Content(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"content", Krys::Move(value));
  }

  DOMString HTMLMetaElement::Media() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"media").value_or(u8"");
  }

  void HTMLMetaElement::Media(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"media", Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLMetaElement Obsolete members

  DOMString HTMLMetaElement::Scheme() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"scheme").value_or(u8"");
  }

  void HTMLMetaElement::Scheme(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"scheme", Krys::Move(value));
  }

#pragma endregion
}
