#include "Krystal.Booey/HTML/HTMLBRElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLBRElement::HTMLBRElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::BR)
  {
  }

#pragma region HTMLBRElement Obsolete members

  dom::DOMString HTMLBRElement::Clear() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"clear").value_or(u8"");
  }

  void HTMLBRElement::Clear(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"clear", krys::move(value));
  }

#pragma endregion
}