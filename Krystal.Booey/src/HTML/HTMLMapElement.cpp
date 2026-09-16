#include "Krystal.Booey/HTML/HTMLMapElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLMapElement::HTMLMapElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Map)
  {
  }

#pragma region HTMLMapElement

  dom::DOMString HTMLMapElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLMapElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

#pragma endregion
}
