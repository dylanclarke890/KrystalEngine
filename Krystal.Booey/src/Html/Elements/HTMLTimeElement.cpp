#include "Krystal.Booey/Html/Elements/HTMLTimeElement.hpp"
#include "Krystal.Booey/Html/Attributes/ReflectAttribute.hpp"

namespace krys::boo
{
  HTMLTimeElement::HTMLTimeElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Time)
  {
  }

#pragma region HTMLTimeElement

  DOMString HTMLTimeElement::DateTime() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"datetime").value_or(u8"");
  }

  void HTMLTimeElement::DateTime(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"datetime", krys::move(value));
  }

#pragma endregion
}
