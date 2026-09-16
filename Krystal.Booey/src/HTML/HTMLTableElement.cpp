#include "Krystal.Booey/HTML/HTMLTableElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLTableElement::HTMLTableElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Table)
  {
  }

#pragma region HTMLTableElement

#pragma endregion

#pragma region HTMLTableElement Obsolete members

  dom::DOMString HTMLTableElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLTableElement::Border() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"border").value_or(u8"");
  }

  void HTMLTableElement::Border(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"border", krys::move(value));
  }

  dom::DOMString HTMLTableElement::Frame() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"frame").value_or(u8"");
  }

  void HTMLTableElement::Frame(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"frame", krys::move(value));
  }

  dom::DOMString HTMLTableElement::Rules() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"rules").value_or(u8"");
  }

  void HTMLTableElement::Rules(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"rules", krys::move(value));
  }

  dom::DOMString HTMLTableElement::Summary() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"summary").value_or(u8"");
  }

  void HTMLTableElement::Summary(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"summary", krys::move(value));
  }

  dom::DOMString HTMLTableElement::Width() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLTableElement::Width(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

  dom::DOMString HTMLTableElement::BgColor() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLTableElement::BgColor(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor", krys::move(value));
  }

  dom::DOMString HTMLTableElement::CellPadding() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"cellpadding").value_or(u8"");
  }

  void HTMLTableElement::CellPadding(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"cellpadding", krys::move(value));
  }

  dom::DOMString HTMLTableElement::CellSpacing() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"cellspacing").value_or(u8"");
  }

  void HTMLTableElement::CellSpacing(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"cellspacing", krys::move(value));
  }

#pragma endregion
}
