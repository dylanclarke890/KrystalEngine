#include "Krystal.HTML/HTML/HTMLLabelElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLLabelElement::HTMLLabelElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Label)
  {
  }

#pragma region HTMLLabelElement

  DOMString HTMLLabelElement::HtmlFor() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"for").value_or(u8"");
  }

  void HTMLLabelElement::HtmlFor(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"for", Krys::Move(value));
  }

#pragma endregion
}