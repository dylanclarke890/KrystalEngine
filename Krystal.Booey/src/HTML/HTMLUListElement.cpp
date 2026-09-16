#include "Krystal.Booey/HTML/HTMLUListElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLUListElement::HTMLUListElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::UList)
  {
  }

#pragma region HTMLUListElement Obsolete members

  bool HTMLUListElement::Compact() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"compact");
  }

  void HTMLUListElement::Compact(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"compact", krys::move(value));
  }

  dom::DOMString HTMLUListElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLUListElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

#pragma endregion
}