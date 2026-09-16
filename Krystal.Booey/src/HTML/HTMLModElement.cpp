#include "Krystal.Booey/HTML/HTMLModElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLModElement::HTMLModElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Mod)
  {
  }

#pragma region HTMLModElement

  dom::ExceptionOr<dom::USVString> HTMLModElement::Cite() const noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"cite");
  }

  dom::ExceptionOr<void> HTMLModElement::Cite(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"cite", krys::move(value));
  }

  dom::DOMString HTMLModElement::DateTime() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"datetime").value_or(u8"");
  }

  void HTMLModElement::DateTime(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"datetime", krys::move(value));
  }

#pragma endregion
}