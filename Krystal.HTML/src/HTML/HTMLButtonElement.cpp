#include "Krystal.HTML/HTML/HTMLButtonElement.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLButtonElement::HTMLButtonElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::Button)
  {
  }

#pragma region HTMLButtonElement

  DOMString HTMLButtonElement::Command() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"command").value_or(u8"");
  }

  void HTMLButtonElement::Command(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"command", Krys::Move(value));
  }

  bool HTMLButtonElement::Disabled() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLButtonElement::Disabled(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"disabled", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLButtonElement::FormAction() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"formaction");
  }

  ExceptionOr<void> HTMLButtonElement::FormAction(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectSetter<USVString>(*this, u8"formaction", Krys::Move(value));
  }

  DOMString HTMLButtonElement::EncType() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"formenctype").value_or(u8"");
  }

  void HTMLButtonElement::EncType(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"formenctype", Krys::Move(value));
  }

  DOMString HTMLButtonElement::FormMethod() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"formmethod").value_or(u8"");
  }

  void HTMLButtonElement::FormMethod(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"formmethod", Krys::Move(value));
  }

  bool HTMLButtonElement::FormNoValidate() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"formnovalidate");
  }

  void HTMLButtonElement::FormNoValidate(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"formnovalidate", Krys::Move(value));
  }

  DOMString HTMLButtonElement::FormTarget() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"formtarget").value_or(u8"");
  }

  void HTMLButtonElement::FormTarget(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"formtarget", Krys::Move(value));
  }

  DOMString HTMLButtonElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLButtonElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

  DOMString HTMLButtonElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLButtonElement::Type(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"type", Krys::Move(value));
  }

  DOMString HTMLButtonElement::Value() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLButtonElement::Value(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"value", Krys::Move(value));
  }

#pragma endregion
}