#include "Krystal.HTML/HTML/HTMLQuoteElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  HTMLQuoteElement::HTMLQuoteElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Quote)
  {
  }

#pragma region HTMLQuoteElement

  ExceptionOr<USVString> HTMLQuoteElement::Cite() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"width");
  }

  ExceptionOr<void> HTMLQuoteElement::Cite(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"width", Krys::Move(value));
  }

#pragma endregion
}