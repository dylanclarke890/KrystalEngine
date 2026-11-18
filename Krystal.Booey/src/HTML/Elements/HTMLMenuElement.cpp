#include "Krystal.Booey/HTML/Elements/HTMLMenuElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo
{
  HTMLMenuElement::HTMLMenuElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Menu)
  {
  }

#pragma region HTMLMenuElement Obsolete members

  bool HTMLMenuElement::Compact() const noexcept
  {
    return html::Reflect<bool>(*this, u8"compact");
  }

  void HTMLMenuElement::Compact(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"compact", krys::move(value));
  }

#pragma endregion
}
