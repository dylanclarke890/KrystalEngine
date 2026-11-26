#include "Krystal.Web/Html/Elements/HTMLModElement.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"

namespace krys::web
{
  HTMLModElement::HTMLModElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Mod)
  {
  }

#pragma region HTMLModElement

  USVString HTMLModElement::Cite() const noexcept
  {
    return html::ReflectUrl<USVString>(*this, u8"cite");
  }

  void HTMLModElement::Cite(USVString value) noexcept
  {
    return html::ReflectUrl<USVString>(*this, u8"cite", krys::move(value));
  }

  DOMString HTMLModElement::DateTime() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"datetime").value_or(u8"");
  }

  void HTMLModElement::DateTime(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"datetime", krys::move(value));
  }

#pragma endregion
}
