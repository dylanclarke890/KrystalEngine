#include "Krystal.Booey/HTML/HTMLTimeElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLTimeElement::HTMLTimeElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Time)
  {
  }

#pragma region HTMLTimeElement

  dom::DOMString HTMLTimeElement::DateTime() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"datetime").value_or(u8"");
  }

  void HTMLTimeElement::DateTime(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"datetime", krys::move(value));
  }

#pragma endregion
}