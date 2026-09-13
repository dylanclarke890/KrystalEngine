#include "Krystal.Booey/HTML/HTMLDListElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLDListElement::HTMLDListElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::DList)
  {
  }

#pragma region HTMLDListElement Obsolete members

  bool HTMLDListElement::Compact() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"compact");
  }

  void HTMLDListElement::Compact(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"compact", krys::move(value));
  }

#pragma endregion
}