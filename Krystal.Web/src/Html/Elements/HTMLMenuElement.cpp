#include "Krystal.Web/Html/Elements/HTMLMenuElement.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"

namespace krys::web
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
