#include "Krystal.HTML/HTML/HTMLDetailsElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLDetailsElement::HTMLDetailsElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Details)
  {
  }

#pragma region HTMLDetailsElement

  DOMString HTMLDetailsElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLDetailsElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

  bool HTMLDetailsElement::Open() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"open");
  }

  void HTMLDetailsElement::Open(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"open", Krys::Move(value));
  }

#pragma endregion
}