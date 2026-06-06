#include "Krystal.HTML/HTML/HTMLHtmlElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLHtmlElement::HTMLHtmlElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Html)
  {
  }

#pragma region HTMLHtmlElement Obsolete members

  DOMString HTMLHtmlElement::Version() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"version").value_or(u8"");
  }

  void HTMLHtmlElement::Version(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"version", Krys::Move(value));
  }

#pragma endregion
}
