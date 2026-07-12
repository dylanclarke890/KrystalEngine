#include "Krystal.HTML/HTML/HTMLEmbedElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
#pragma region HTMLEmbedElement

  HTMLEmbedElement::HTMLEmbedElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Embed)
  {
  }

  ExceptionOr<USVString> HTMLEmbedElement::Src() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"src");
  }

  ExceptionOr<void> HTMLEmbedElement::Src(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"src", Krys::Move(value));
  }

  DOMString HTMLEmbedElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLEmbedElement::Type(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"type", Krys::Move(value));
  }

  DOMString HTMLEmbedElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLEmbedElement::Width(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"width", Krys::Move(value));
  }

  DOMString HTMLEmbedElement::Height() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"height").value_or(u8"");
  }

  void HTMLEmbedElement::Height(DOMString value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"height", Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLEmbedElement Obsolete members

  DOMString HTMLEmbedElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLEmbedElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

  DOMString HTMLEmbedElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLEmbedElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

#pragma endregion
}
