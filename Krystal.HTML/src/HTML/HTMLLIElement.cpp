#include "Krystal.HTML/HTML/HTMLLIElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLLIElement::HTMLLIElement(Document &document) noexcept : HTMLElement(document, DOMInterface::LI)
  {
  }

#pragma region HTMLLIElement

  int32 HTMLLIElement::Value() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"value");
  }

  void HTMLLIElement::Value(int32 value) noexcept
  {
    Attributes::Reflection::Reflect<int32>(*this, u8"value", Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLLIElement Obsolete members

  DOMString HTMLLIElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLLIElement::Type(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"type", Krys::Move(value));
  }

#pragma endregion
}