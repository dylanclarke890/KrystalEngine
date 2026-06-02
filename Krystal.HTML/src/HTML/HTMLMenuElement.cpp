#include "Krystal.HTML/HTML/HTMLMenuElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLMenuElement::HTMLMenuElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Menu)
  {
  }

#pragma region HTMLMenuElement Obsolete members

  bool HTMLMenuElement::Compact() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"compact");
  }

  void HTMLMenuElement::Compact(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"compact", Krys::Move(value));
  }

#pragma endregion
}