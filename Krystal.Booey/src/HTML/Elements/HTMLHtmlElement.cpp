#include "Krystal.Booey/HTML/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  HTMLHtmlElement::HTMLHtmlElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Html)
  {
  }

#pragma region HTMLHtmlElement Obsolete members

  dom::DOMString HTMLHtmlElement::Version() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"version").value_or(u8"");
  }

  void HTMLHtmlElement::Version(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"version", krys::move(value));
  }

#pragma endregion
}
