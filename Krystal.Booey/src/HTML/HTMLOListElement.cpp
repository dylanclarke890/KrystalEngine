#include "Krystal.Booey/HTML/HTMLOListElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
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
    Attributes::Reflection::Reflect<bool>(*this, u8"reversed", krys::move(value));
  }

  int32 HTMLOListElement::Start() const noexcept
  {
    return Attributes::Reflection::Reflect<int32, Attributes::ReflectDefault<int32>(1)>(*this, u8"start");
  }

  void HTMLOListElement::Start(int32 value) noexcept
  {
    Attributes::Reflection::Reflect<int32>(*this, u8"start", krys::move(value));
  }

  dom::DOMString HTMLOListElement::Type() const noexcept
  {
    // TODO(HTMLOLISTLEMENT, HTML): Type is an enumerated attribute.
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLOListElement::Type(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

#pragma endregion

#pragma region HTMLOListElement Obsolete members

  bool HTMLOListElement::Compact() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"compact");
  }

  void HTMLOListElement::Compact(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"compact", krys::move(value));
  }

#pragma endregion
}