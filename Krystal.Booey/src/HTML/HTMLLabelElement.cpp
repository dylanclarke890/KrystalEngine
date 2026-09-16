#include "Krystal.Booey/HTML/HTMLLabelElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLLabelElement::HTMLLabelElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Label)
  {
  }

#pragma region HTMLLabelElement

  dom::DOMString HTMLLabelElement::HtmlFor() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"for").value_or(u8"");
  }

  void HTMLLabelElement::HtmlFor(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"for", krys::move(value));
  }

#pragma endregion
}