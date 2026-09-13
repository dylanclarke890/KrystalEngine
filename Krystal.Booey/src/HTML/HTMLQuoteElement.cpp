#include "Krystal.Booey/HTML/HTMLQuoteElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLQuoteElement::HTMLQuoteElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Quote)
  {
  }

#pragma region HTMLQuoteElement

  dom::ExceptionOr<dom::USVString> HTMLQuoteElement::Cite() const noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"cite");
  }

  dom::ExceptionOr<void> HTMLQuoteElement::Cite(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"cite", krys::move(value));
  }

#pragma endregion
}