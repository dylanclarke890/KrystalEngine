#include "Krystal.HTML/HTML/HTMLUListElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLUListElement::HTMLUListElement(Document &document) noexcept : HTMLElement(document, DOMInterface::UList)
  {
  }

#pragma region HTMLUListElement Obsolete members

  bool HTMLUListElement::Compact() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"compact");
  }

  void HTMLUListElement::Compact(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"compact", Krys::Move(value));
  }

  DOMString HTMLUListElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLUListElement::Type(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"type", Krys::Move(value));
  }

#pragma endregion
}