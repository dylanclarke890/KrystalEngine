#include "Krystal.HTML/HTML/HTMLTableRowElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLTableRowElement::HTMLTableRowElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::TableRow)
  {
  }

#pragma region HTMLTableRowElement Obsolete members

  DOMString HTMLTableRowElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableRowElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

  DOMString HTMLTableRowElement::Char() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableRowElement::Char(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"char", Krys::Move(value));
  }

  DOMString HTMLTableRowElement::CharOff() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableRowElement::CharOff(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"charoff", Krys::Move(value));
  }

  DOMString HTMLTableRowElement::VAlign() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableRowElement::VAlign(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"valign", Krys::Move(value));
  }

  DOMString HTMLTableRowElement::BgColor() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLTableRowElement::BgColor(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"bgcolor", Krys::Move(value));
  }

#pragma endregion
}
