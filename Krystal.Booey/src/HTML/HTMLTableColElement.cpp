#include "Krystal.Booey/HTML/HTMLTableColElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLTableColElement::HTMLTableColElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::TableCol)
  {
  }

#pragma region HTMLTableColElement

  uint32 HTMLTableColElement::Span() const noexcept
  {
    return Reflection::Reflect<uint32, ReflectDefault<uint32> {1}, ReflectRange<uint32> {1u, 1'000u}>(
      *this, u8"span");
  }

  void HTMLTableColElement::Span(uint32 value) noexcept
  {
    Reflection::Reflect<uint32, ReflectDefault<uint32> {1}>(*this, u8"span", krys::move(value));
  }

#pragma endregion

#pragma region HTMLTableColElement Obsolete members

  dom::DOMString HTMLTableColElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableColElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLTableColElement::Char() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableColElement::Char(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"char", krys::move(value));
  }

  dom::DOMString HTMLTableColElement::CharOff() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableColElement::CharOff(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"charoff", krys::move(value));
  }

  dom::DOMString HTMLTableColElement::VAlign() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableColElement::VAlign(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"valign", krys::move(value));
  }

  dom::DOMString HTMLTableColElement::Width() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLTableColElement::Width(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

#pragma endregion
}
