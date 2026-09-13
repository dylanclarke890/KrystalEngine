#include "Krystal.Booey/HTML/HTMLLegendElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLLegendElement::HTMLLegendElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Legend)
  {
  }

#pragma region HTMLLegendElement

#pragma endregion

#pragma region HTMLLegendElement Obsolete members

  dom::DOMString HTMLLegendElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLLegendElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

#pragma endregion
}