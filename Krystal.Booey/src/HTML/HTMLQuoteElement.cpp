#include "Krystal.Booey/HTML/HTMLQuoteElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLQuoteElement::HTMLQuoteElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Quote)
  {
  }

#pragma region HTMLQuoteElement

  dom::ExceptionOr<USVString> HTMLQuoteElement::Cite() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"cite");
  }

  dom::ExceptionOr<void> HTMLQuoteElement::Cite(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"cite", krys::move(value));
  }

#pragma endregion
}