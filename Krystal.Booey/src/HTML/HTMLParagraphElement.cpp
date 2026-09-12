#include "Krystal.Booey/HTML/HTMLParagraphElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLParagraphElement::HTMLParagraphElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Heading)
  {
  }

#pragma region HTMLHeadingElement Obsolete members

  dom::DOMString HTMLParagraphElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLParagraphElement::Align(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

#pragma endregion
}