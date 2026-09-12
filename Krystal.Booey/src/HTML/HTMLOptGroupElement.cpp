#include "Krystal.Booey/HTML/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLOptGroupElement::HTMLOptGroupElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::OptGroup)
  {
  }

#pragma region HTMLOptGroupElement

  bool HTMLOptGroupElement::Disabled() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLOptGroupElement::Disabled(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::DOMString HTMLOptGroupElement::Label() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"label").value_or(u8"");
  }

  void HTMLOptGroupElement::Label(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"label", krys::move(value));
  }

#pragma endregion
}