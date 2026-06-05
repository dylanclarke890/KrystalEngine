#include "Krystal.HTML/HTML/HTMLBodyElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  HTMLBodyElement::HTMLBodyElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Body)
  {
  }

#pragma region HTMLBodyElement Obsolete members

  DOMString HTMLBodyElement::Text() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"text").value_or(u8"");
  }

  void HTMLBodyElement::Text(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"text", Krys::Move(value));
  }

  DOMString HTMLBodyElement::Link() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"link").value_or(u8"");
  }

  void HTMLBodyElement::Link(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"link", Krys::Move(value));
  }

  DOMString HTMLBodyElement::VLink() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"vlink").value_or(u8"");
  }

  void HTMLBodyElement::VLink(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"vlink", Krys::Move(value));
  }

  DOMString HTMLBodyElement::ALink() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"alink").value_or(u8"");
  }

  void HTMLBodyElement::ALink(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"alink", Krys::Move(value));
  }

  DOMString HTMLBodyElement::BgColor() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"bgcolor").value_or(u8"");
  }

  void HTMLBodyElement::BgColor(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"bgcolor", Krys::Move(value));
  }

  DOMString HTMLBodyElement::Background() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"background").value_or(u8"");
  }

  void HTMLBodyElement::Background(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"background", Krys::Move(value));
  }

#pragma endregion
}
