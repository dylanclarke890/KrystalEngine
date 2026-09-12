#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLHtmlElement::HTMLHtmlElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Html)
  {
  }

#pragma region HTMLHtmlElement Obsolete members

  dom::DOMString HTMLHtmlElement::Version() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"version").value_or(u8"");
  }

  void HTMLHtmlElement::Version(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"version", krys::move(value));
  }

#pragma endregion
}
