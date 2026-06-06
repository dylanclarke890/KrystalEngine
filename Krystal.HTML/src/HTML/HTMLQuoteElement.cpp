#include "Krystal.HTML/HTML/HTMLQuoteElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLQuoteElement::HTMLQuoteElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Quote)
  {
  }

#pragma region HTMLQuoteElement

  ExceptionOr<USVString> HTMLQuoteElement::Cite() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"cite");
  }

  ExceptionOr<void> HTMLQuoteElement::Cite(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"cite", Krys::Move(value));
  }

#pragma endregion
}