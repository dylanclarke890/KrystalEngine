#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/HTMLDataElement.hpp"

namespace krys::boo::html
{
  HTMLDataElement::HTMLDataElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Data)
  {
  }

#pragma region HTMLDataElement

  dom::DOMString HTMLDataElement::Value() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLDataElement::Value(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"value", krys::move(value));
  }

#pragma endregion
}