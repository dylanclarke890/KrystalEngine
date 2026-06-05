#include "Krystal.HTML/HTML/HTMLBRElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLBRElement::HTMLBRElement(Document &document) noexcept : HTMLElement(document, DOMInterface::BR)
  {
  }

#pragma region HTMLBRElement Obsolete members

  DOMString HTMLBRElement::Clear() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"clear").value_or(u8"");
  }

  void HTMLBRElement::Clear(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"clear", Krys::Move(value));
  }

#pragma endregion
}