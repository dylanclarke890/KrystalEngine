#include "Krystal.HTML/HTML/HTMLFieldSetElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
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
    Attributes::Reflection::Reflect<bool>(*this, u8"disabled", Krys::Move(value));
  }

  DOMString HTMLFieldSetElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLFieldSetElement::Name(DOMString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

#pragma endregion
}