#include "Krystal.Booey/HTML/Elements/HTMLQuoteElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  HTMLQuoteElement::HTMLQuoteElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Quote)
  {
  }

#pragma region HTMLQuoteElement

  dom::USVString HTMLQuoteElement::Cite() const noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"cite");
  }

  void HTMLQuoteElement::Cite(dom::USVString value) noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"cite", krys::move(value));
  }

#pragma endregion
}