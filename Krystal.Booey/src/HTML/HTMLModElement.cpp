#include "Krystal.Booey/HTML/HTMLModElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

namespace krys::boo::html
{
  HTMLModElement::HTMLModElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Mod)
  {
  }

#pragma region HTMLModElement

  dom::USVString HTMLModElement::Cite() const noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"cite");
  }

  void HTMLModElement::Cite(dom::USVString &&value) noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"cite", krys::move(value));
  }

  dom::DOMString HTMLModElement::DateTime() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"datetime").value_or(u8"");
  }

  void HTMLModElement::DateTime(dom::DOMString &&value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"datetime", krys::move(value));
  }

#pragma endregion
}