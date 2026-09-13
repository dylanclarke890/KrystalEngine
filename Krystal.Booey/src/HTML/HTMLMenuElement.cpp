#include "Krystal.Booey/HTML/HTMLMenuElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLMenuElement::HTMLMenuElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Menu)
  {
  }

#pragma region HTMLMenuElement Obsolete members

  bool HTMLMenuElement::Compact() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"compact");
  }

  void HTMLMenuElement::Compact(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"compact", krys::move(value));
  }

#pragma endregion
}