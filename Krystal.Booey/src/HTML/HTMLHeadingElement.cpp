#include "Krystal.Booey/HTML/HTMLHeadingElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  HTMLHeadingElement::HTMLHeadingElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Heading)
  {
  }

#pragma region HTMLHeadingElement Obsolete members

  dom::DOMString HTMLHeadingElement::Align() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLHeadingElement::Align(dom::DOMString &&value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

#pragma endregion
}