#include "Krystal.Booey/HTML/HTMLHRElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLHRElement::HTMLHRElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::HR)
  {
  }

#pragma region HTMLHRElement Obsolete members

  dom::DOMString HTMLHRElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLHRElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

  dom::DOMString HTMLHRElement::Color() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"color").value_or(u8"");
  }

  void HTMLHRElement::Color(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"color", krys::move(value));
  }

  bool HTMLHRElement::NoShade() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"noshade");
  }

  void HTMLHRElement::NoShade(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"noshade", krys::move(value));
  }

  dom::DOMString HTMLHRElement::Size() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"size").value_or(u8"");
  }

  void HTMLHRElement::Size(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"size", krys::move(value));
  }

  dom::DOMString HTMLHRElement::Width() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLHRElement::Width(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"width", krys::move(value));
  }

#pragma endregion
}