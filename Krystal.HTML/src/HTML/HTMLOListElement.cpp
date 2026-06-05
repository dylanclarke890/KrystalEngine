#include "Krystal.HTML/HTML/HTMLOListElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  HTMLOListElement::HTMLOListElement(Document &document) noexcept : HTMLElement(document, DOMInterface::OList)
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