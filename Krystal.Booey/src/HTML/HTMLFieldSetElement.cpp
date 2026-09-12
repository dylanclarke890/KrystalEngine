#include "Krystal.Booey/HTML/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLFieldSetElement::HTMLFieldSetElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::FieldSet)
  {
  }

#pragma region HTMLFieldSetElement

  bool HTMLFieldSetElement::Disabled() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLFieldSetElement::Disabled(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::DOMString HTMLFieldSetElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLFieldSetElement::Name(dom::DOMString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

#pragma endregion
}