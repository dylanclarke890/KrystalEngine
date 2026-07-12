#include "Krystal.HTML/HTML/HTMLTableElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLTableElement::HTMLTableElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Table)
  {
  }

#pragma region HTMLTableElement

#pragma endregion

#pragma region HTMLTableElement Obsolete members

  DOMString HTMLTableElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

  DOMString HTMLTableElement::Border() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLTableElement::Border(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"border", Krys::Move(value));
  }

  DOMString HTMLTableElement::Frame() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"frame").value_or(u8"");
  }

  void HTMLTableElement::Frame(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"frame", Krys::Move(value));
  }

  DOMString HTMLTableElement::Rules() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"rules").value_or(u8"");
  }

  void HTMLTableElement::Rules(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"rules", Krys::Move(value));
  }

  DOMString HTMLTableElement::Summary() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"summary").value_or(u8"");
  }

  void HTMLTableElement::Summary(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"summary", Krys::Move(value));
  }

  DOMString HTMLTableElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLTableElement::Width(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"width", Krys::Move(value));
  }

  DOMString HTMLTableElement::BgColor() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLTableElement::BgColor(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"bgcolor", Krys::Move(value));
  }

  DOMString HTMLTableElement::CellPadding() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"cellpadding").value_or(u8"");
  }

  void HTMLTableElement::CellPadding(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"cellpadding", Krys::Move(value));
  }

  DOMString HTMLTableElement::CellSpacing() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"cellspacing").value_or(u8"");
  }

  void HTMLTableElement::CellSpacing(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"cellspacing", Krys::Move(value));
  }

#pragma endregion
}
