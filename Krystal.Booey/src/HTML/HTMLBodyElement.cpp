#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLBodyElement::HTMLBodyElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Body)
  {
  }

#pragma region HTMLBodyElement Obsolete members

  dom::DOMString HTMLBodyElement::Text() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"text").value_or(u8"");
  }

  void HTMLBodyElement::Text(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"text", krys::move(value));
  }

  dom::DOMString HTMLBodyElement::Link() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"link").value_or(u8"");
  }

  void HTMLBodyElement::Link(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"link", krys::move(value));
  }

  dom::DOMString HTMLBodyElement::VLink() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"vlink").value_or(u8"");
  }

  void HTMLBodyElement::VLink(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"vlink", krys::move(value));
  }

  dom::DOMString HTMLBodyElement::ALink() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"alink").value_or(u8"");
  }

  void HTMLBodyElement::ALink(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"alink", krys::move(value));
  }

  dom::DOMString HTMLBodyElement::BgColor() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLBodyElement::BgColor(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"bgcolor", krys::move(value));
  }

  dom::DOMString HTMLBodyElement::Background() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"background").value_or(u8"");
  }

  void HTMLBodyElement::Background(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"background", krys::move(value));
  }

#pragma endregion
}
