#include "Krystal.Booey/HTML/HTMLDataElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  HTMLDataElement::HTMLDataElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Data)
  {
  }

#pragma region HTMLDataElement

  dom::DOMString HTMLDataElement::Value() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLDataElement::Value(dom::DOMString &&value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"value", krys::move(value));
  }

#pragma endregion
}