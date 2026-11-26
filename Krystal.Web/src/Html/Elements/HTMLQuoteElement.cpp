#include "Krystal.Web/Html/Elements/HTMLQuoteElement.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"

namespace krys::web
{
  HTMLQuoteElement::HTMLQuoteElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Quote)
  {
  }

#pragma region HTMLQuoteElement

  USVString HTMLQuoteElement::Cite() const noexcept
  {
    return html::ReflectUrl<USVString>(*this, u8"cite");
  }

  void HTMLQuoteElement::Cite(USVString value) noexcept
  {
    return html::ReflectUrl<USVString>(*this, u8"cite", krys::move(value));
  }

#pragma endregion
}
