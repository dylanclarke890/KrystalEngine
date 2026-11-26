#include "Krystal.Web/Html/Elements/HTMLQuoteElement.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"

namespace krys::web
{
  HTMLQuoteElement::HTMLQuoteElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::Quote)
  {
  }

#pragma region HTMLQuoteElement

  dom::USVString HTMLQuoteElement::Cite() const noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"cite");
  }

  void HTMLQuoteElement::Cite(dom::USVString value) noexcept
  {
    return html::ReflectUrl<dom::USVString>(*this, u8"cite", krys::move(value));
  }

#pragma endregion
}
