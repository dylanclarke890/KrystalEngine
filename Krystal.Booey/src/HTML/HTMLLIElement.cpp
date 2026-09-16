#include "Krystal.Booey/HTML/HTMLLIElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLLIElement::HTMLLIElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::LI)
  {
  }

#pragma region HTMLLIElement

  int32 HTMLLIElement::Value() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"value");
  }

  void HTMLLIElement::Value(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"value", krys::move(value));
  }

#pragma endregion

#pragma region HTMLLIElement Obsolete members

  dom::DOMString HTMLLIElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLLIElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

#pragma endregion
}