#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/HTMLDListElement.hpp"

namespace Krys::HTML
{
  HTMLDListElement::HTMLDListElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::DList)
  {
  }

#pragma region HTMLDListElement Obsolete members

  bool HTMLDListElement::Compact() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"compact");
  }

  void HTMLDListElement::Compact(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"compact", Krys::Move(value));
  }

#pragma endregion
}