#include "Krystal.HTML/HTML/HTMLTableCellElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLTableCellElement::HTMLTableCellElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::TableCell)
  {
  }

#pragma region HTMLTableCellElement

  uint32 HTMLTableCellElement::ColSpan() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32, Attributes::ReflectDefault<uint32> {1},
                                           Attributes::ReflectRange<uint32> {1, 1'000}>(*this, u8"colspan");
  }

  void HTMLTableCellElement::ColSpan(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32, Attributes::ReflectDefault<uint32> {1}>(*this, u8"colspan",
                                                                                    value);
  }

  uint32 HTMLTableCellElement::RowSpan() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32, Attributes::ReflectDefault<uint32> {1},
                                           Attributes::ReflectRange<uint32> {1, 65'534}>(*this, u8"rowspan");
  }

  void HTMLTableCellElement::RowSpan(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32, Attributes::ReflectDefault<uint32> {1}>(*this, u8"rowspan",
                                                                                    value);
  }

  DOMString HTMLTableCellElement::Headers() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"headers").value_or(u8"");
  }

  void HTMLTableCellElement::Headers(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"headers", Krys::Move(value));
  }

  DOMString HTMLTableCellElement::Scope() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"scope").value_or(u8"");
  }

  void HTMLTableCellElement::Scope(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"scope", Krys::Move(value));
  }

  DOMString HTMLTableCellElement::Abbr() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"abbr").value_or(u8"");
  }

  void HTMLTableCellElement::Abbr(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"abbr", Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLTableCellElement Obsolete members

  DOMString HTMLTableCellElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableCellElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

  DOMString HTMLTableCellElement::Axis() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"axis").value_or(u8"");
  }

  void HTMLTableCellElement::Axis(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"axis", Krys::Move(value));
  }

  DOMString HTMLTableCellElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLTableCellElement::Height(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"height", Krys::Move(value));
  }

  DOMString HTMLTableCellElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLTableCellElement::Width(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"width", Krys::Move(value));
  }

  DOMString HTMLTableCellElement::Char() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableCellElement::Char(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"char", Krys::Move(value));
  }

  DOMString HTMLTableCellElement::CharOff() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableCellElement::CharOff(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"charoff", Krys::Move(value));
  }

  bool HTMLTableCellElement::NoWrap() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"nowrap");
  }

  void HTMLTableCellElement::NoWrap(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"nowrap", Krys::Move(value));
  }

  DOMString HTMLTableCellElement::VAlign() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableCellElement::VAlign(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"valign", Krys::Move(value));
  }

  DOMString HTMLTableCellElement::BgColor() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLTableCellElement::BgColor(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"bgcolor", Krys::Move(value));
  }

#pragma endregion
}