#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/HTMLModElement.hpp"

namespace Krys::HTML
{
  HTMLModElement::HTMLModElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Mod)
  {
  }

#pragma region HTMLModElement

  ExceptionOr<USVString> HTMLModElement::Cite() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"cite");
  }

  ExceptionOr<void> HTMLModElement::Cite(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"cite", Krys::Move(value));
  }

  
  DOMString HTMLModElement::DateTime() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"datetime").value_or(u8"");
  }

  void HTMLModElement::DateTime(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"datetime", Krys::Move(value));
  }

#pragma endregion
}