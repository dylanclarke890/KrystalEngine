#include "Krystal.HTML/HTML/HTMLDivElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLDivElement::HTMLDivElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Div)
  {
  }

#pragma region HTMLDivElement Obsolete members

  DOMString HTMLDivElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLDivElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

#pragma endregion
}