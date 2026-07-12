#include "Krystal.HTML/HTML/HTMLTimeElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLTimeElement::HTMLTimeElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Time)
  {
  }

#pragma region HTMLTimeElement

  DOMString HTMLTimeElement::DateTime() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"datetime").value_or(u8"");
  }

  void HTMLTimeElement::DateTime(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"datetime", Krys::Move(value));
  }

#pragma endregion
}