#include "Krystal.HTML/HTML/HTMLOListElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLOListElement::HTMLOListElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::OList)
  {
  }

#pragma region HTMLOListElement

  bool HTMLOListElement::Reversed() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"reversed");
  }

  void HTMLOListElement::Reversed(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"reversed", Krys::Move(value));
  }

  int32 HTMLOListElement::Start() const noexcept
  {
    return Attributes::Reflection::Reflect<int32, Attributes::ReflectDefault<int32>(1)>(*this, u8"start");
  }

  void HTMLOListElement::Start(int32 value) noexcept
  {
    Attributes::Reflection::Reflect<int32>(*this, u8"start", Krys::Move(value));
  }

  DOMString HTMLOListElement::Type() const noexcept
  {
    // TODO(HTMLOLISTLEMENT, HTML): Type is an enumerated attribute.
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLOListElement::Type(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"type", Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLOListElement Obsolete members

  bool HTMLOListElement::Compact() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"compact");
  }

  void HTMLOListElement::Compact(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"compact", Krys::Move(value));
  }

#pragma endregion
}