#include "Krystal.Booey/Html/Elements/HTMLDListElement.hpp"
#include "Krystal.Booey/Html/Attributes/ReflectAttribute.hpp"

namespace krys::boo
{
  HTMLDListElement::HTMLDListElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::DList)
  {
  }

#pragma region HTMLDListElement Obsolete members

  bool HTMLDListElement::Compact() const noexcept
  {
    return html::Reflect<bool>(*this, u8"compact");
  }

  void HTMLDListElement::Compact(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"compact", krys::move(value));
  }

#pragma endregion
}
