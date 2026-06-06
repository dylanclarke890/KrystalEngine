#include "Krystal.HTML/HTML/HTMLBaseElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLBaseElement::HTMLBaseElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Base)
  {
  }

#pragma region HTMLBaseElement

  ExceptionOr<USVString> HTMLBaseElement::Href() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"href");
  }

  ExceptionOr<void> HTMLBaseElement::Href(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectSetter<USVString>(*this, u8"href", Krys::Move(value));
  }

  DOMString HTMLBaseElement::Target() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLBaseElement::Target(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"target", Krys::Move(value));
  }

#pragma endregion
}
