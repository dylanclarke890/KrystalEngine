#include "Krystal.HTML/HTML/HTMLTableCaptionElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLTableCaptionElement::HTMLTableCaptionElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::TableCaption)
  {
  }

#pragma region HTMLTableCaptionElement Obsolete members

  DOMString HTMLTableCaptionElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableCaptionElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

#pragma endregion
}
