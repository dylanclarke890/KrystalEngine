#include "Krystal.Booey/HTML/HTMLDivElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLDivElement::HTMLDivElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Div)
  {
  }

#pragma region HTMLDivElement Obsolete members

  dom::DOMString HTMLDivElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLDivElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

#pragma endregion
}