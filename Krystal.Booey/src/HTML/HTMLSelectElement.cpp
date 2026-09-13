#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLSelectElement::HTMLSelectElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Select)
  {
  }

#pragma region HTMLSelectElement

  dom::DOMString HTMLSelectElement::Autocomplete() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLSelectElement::Autocomplete(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  bool HTMLSelectElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLSelectElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  bool HTMLSelectElement::Multiple() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"multiple");
  }

  void HTMLSelectElement::Multiple(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"multiple", krys::move(value));
  }

  dom::DOMString HTMLSelectElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLSelectElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  bool HTMLSelectElement::Required() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"required");
  }

  void HTMLSelectElement::Required(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"required", krys::move(value));
  }

  uint32 HTMLSelectElement::Size() const noexcept
  {
    return Reflection::Reflect<uint32>(*this, u8"size");
  }

  void HTMLSelectElement::Size(uint32 value) noexcept
  {
    Reflection::Reflect<uint32>(*this, u8"size", krys::move(value));
  }

#pragma endregion
}