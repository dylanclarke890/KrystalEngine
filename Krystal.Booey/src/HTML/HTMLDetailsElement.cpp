#include "Krystal.Booey/HTML/HTMLDetailsElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLDetailsElement::HTMLDetailsElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Details)
  {
  }

#pragma region HTMLDetailsElement

  dom::DOMString HTMLDetailsElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLDetailsElement::Name(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  bool HTMLDetailsElement::Open() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"open");
  }

  void HTMLDetailsElement::Open(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"open", krys::move(value));
  }

#pragma endregion
}