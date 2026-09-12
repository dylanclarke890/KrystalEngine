#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/HTMLDListElement.hpp"

namespace krys::boo::html
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
    Attributes::Reflection::Reflect<bool>(*this, u8"compact", krys::move(value));
  }

#pragma endregion
}