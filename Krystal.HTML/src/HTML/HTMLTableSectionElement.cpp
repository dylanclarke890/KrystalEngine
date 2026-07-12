#include "Krystal.HTML/HTML/HTMLTableSectionElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLTableSectionElement::HTMLTableSectionElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::TableSection)
  {
  }

#pragma region HTMLTableSectionElement Obsolete members

  DOMString HTMLTableSectionElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableSectionElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

  DOMString HTMLTableSectionElement::Char() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableSectionElement::Char(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"char", Krys::Move(value));
  }

  DOMString HTMLTableSectionElement::CharOff() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableSectionElement::CharOff(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"charoff", Krys::Move(value));
  }

  DOMString HTMLTableSectionElement::VAlign() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableSectionElement::VAlign(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"valign", Krys::Move(value));
  }

#pragma endregion
}
