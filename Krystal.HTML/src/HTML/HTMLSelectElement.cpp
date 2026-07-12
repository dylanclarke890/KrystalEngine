#include "Krystal.HTML/HTML/HTMLSelectElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLSelectElement::HTMLSelectElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Select)
  {
  }

#pragma region HTMLSelectElement

  DOMString HTMLSelectElement::Autocomplete() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLSelectElement::Autocomplete(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"autocomplete", Krys::Move(value));
  }

  bool HTMLSelectElement::Disabled() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLSelectElement::Disabled(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"disabled", Krys::Move(value));
  }

  bool HTMLSelectElement::Multiple() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"multiple");
  }

  void HTMLSelectElement::Multiple(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"multiple", Krys::Move(value));
  }

  DOMString HTMLSelectElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLSelectElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

  bool HTMLSelectElement::Required() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"required");
  }

  void HTMLSelectElement::Required(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"required", Krys::Move(value));
  }

  uint32 HTMLSelectElement::Size() const noexcept
  {
    return Attributes::Reflection::Reflect<uint32>(*this, u8"size");
  }

  void HTMLSelectElement::Size(uint32 value) noexcept
  {
    Attributes::Reflection::Reflect<uint32>(*this, u8"size", Krys::Move(value));
  }

#pragma endregion
}