#include "Krystal.HTML/HTML/HTMLHRElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  HTMLHRElement::HTMLHRElement(Document &document) noexcept : HTMLElement(document, DOMInterface::HR)
  {
  }

#pragma region HTMLHRElement Obsolete members

  DOMString HTMLHRElement::Align() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLHRElement::Align(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"align", Krys::Move(value));
  }

  DOMString HTMLHRElement::Color() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"color").value_or(u8"");
  }

  void HTMLHRElement::Color(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"color", Krys::Move(value));
  }

  bool HTMLHRElement::NoShade() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"noshade");
  }

  void HTMLHRElement::NoShade(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"noshade", Krys::Move(value));
  }

  DOMString HTMLHRElement::Size() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"size").value_or(u8"");
  }

  void HTMLHRElement::Size(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"size", Krys::Move(value));
  }

  DOMString HTMLHRElement::Width() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"width").value_or(u8"");
  }

  void HTMLHRElement::Width(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"width", Krys::Move(value));
  }

#pragma endregion
}