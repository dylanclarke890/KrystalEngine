#include "Krystal.HTML/HTML/HTMLTableColElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLTableColElement::HTMLTableColElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::TableCol)
  {
  }

#pragma region HTMLTableColElement

  uint32 HTMLTableColElement::Span() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32, Attributes::ReflectDefault<uint32> {1},
                                           Attributes::ReflectRange<uint32> {1u, 1'000u}>(*this, u8"span");
  }

  void HTMLTableColElement::Span(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32, Attributes::ReflectDefault<uint32> {1}>(*this, u8"span",
                                                                                    Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLTableColElement Obsolete members

  DOMString HTMLTableColElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableColElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

  DOMString HTMLTableColElement::Char() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"char").value_or(u8"");
  }

  void HTMLTableColElement::Char(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"char", Krys::Move(value));
  }

  DOMString HTMLTableColElement::CharOff() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"charoff").value_or(u8"");
  }

  void HTMLTableColElement::CharOff(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"charoff", Krys::Move(value));
  }

  DOMString HTMLTableColElement::VAlign() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"valign").value_or(u8"");
  }

  void HTMLTableColElement::VAlign(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"valign", Krys::Move(value));
  }

  DOMString HTMLTableColElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLTableColElement::Width(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"width", Krys::Move(value));
  }

#pragma endregion
}
