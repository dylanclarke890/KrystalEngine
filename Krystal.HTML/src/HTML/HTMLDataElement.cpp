#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/HTMLDataElement.hpp"

namespace Krys::HTML
{
  HTMLDataElement::HTMLDataElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Data)
  {
  }

#pragma region HTMLDataElement

  DOMString HTMLDataElement::Value() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLDataElement::Value(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"value", Krys::Move(value));
  }

#pragma endregion
}