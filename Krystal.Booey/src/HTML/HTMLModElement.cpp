#include "Krystal.Booey/HTML/HTMLModElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLModElement::HTMLModElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Mod)
  {
  }

#pragma region HTMLModElement

  dom::ExceptionOr<USVString> HTMLModElement::Cite() const noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"cite");
  }

  dom::ExceptionOr<void> HTMLModElement::Cite(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectURL<USVString>(*this, u8"cite", krys::move(value));
  }

  dom::DOMString HTMLModElement::DateTime() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"datetime").value_or(u8"");
  }

  void HTMLModElement::DateTime(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"datetime", krys::move(value));
  }

#pragma endregion
}