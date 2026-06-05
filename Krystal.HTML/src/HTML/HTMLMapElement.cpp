#include "Krystal.HTML/HTML/HTMLMapElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLMapElement::HTMLMapElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Map)
  {
  }

#pragma region HTMLMapElement

  DOMString HTMLMapElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLMapElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

#pragma endregion
}
