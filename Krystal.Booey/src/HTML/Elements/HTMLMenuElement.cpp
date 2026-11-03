#include "Krystal.Booey/HTML/Elements/HTMLMenuElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  HTMLMenuElement::HTMLMenuElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Menu)
  {
  }

#pragma region HTMLMenuElement Obsolete members

  bool HTMLMenuElement::Compact() const noexcept
  {
    return Reflect<bool>(*this, u8"compact");
  }

  void HTMLMenuElement::Compact(bool value) noexcept
  {
    Reflect<bool>(*this, u8"compact", krys::move(value));
  }

#pragma endregion
}