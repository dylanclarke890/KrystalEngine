#include "Krystal.Booey/HTML/HTMLTableCellElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLTableCellElement::HTMLTableCellElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::TableCell)
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

  dom::DOMString HTMLTableCellElement::Headers() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"headers").value_or(u8"");
  }

  void HTMLTableCellElement::Headers(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"headers", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::Scope() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"scope").value_or(u8"");
  }

  void HTMLTableCellElement::Scope(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"scope", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::Abbr() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"abbr").value_or(u8"");
  }

  void HTMLTableCellElement::Abbr(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"abbr", krys::move(value));
  }

#pragma endregion

#pragma region HTMLTableCellElement Obsolete members

  dom::DOMString HTMLTableCellElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableCellElement::Align(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::Axis() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"axis").value_or(u8"");
  }

  void HTMLTableCellElement::Axis(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"axis", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLTableCellElement::Height(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"height", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLTableCellElement::Width(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::Char() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableCellElement::Char(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"char", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::CharOff() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableCellElement::CharOff(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"charoff", krys::move(value));
  }

  bool HTMLTableCellElement::NoWrap() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"nowrap");
  }

  void HTMLTableCellElement::NoWrap(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"nowrap", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::VAlign() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableCellElement::VAlign(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"valign", krys::move(value));
  }

  dom::DOMString HTMLTableCellElement::BgColor() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLTableCellElement::BgColor(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor", krys::move(value));
  }

#pragma endregion
}