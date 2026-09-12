#include "Krystal.Booey/HTML/HTMLTableCaptionElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLTableCaptionElement::HTMLTableCaptionElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::TableCaption)
  {
  }

#pragma region HTMLTableCaptionElement Obsolete members

  dom::DOMString HTMLTableCaptionElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableCaptionElement::Align(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

#pragma endregion
}
