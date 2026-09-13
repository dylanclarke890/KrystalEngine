#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLButtonElement::HTMLButtonElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Button)
  {
  }

#pragma region HTMLButtonElement

  dom::DOMString HTMLButtonElement::Command() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"command").value_or(u8"");
  }

  void HTMLButtonElement::Command(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"command", krys::move(value));
  }

  bool HTMLButtonElement::Disabled() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"disabled");
  }

  void HTMLButtonElement::Disabled(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"disabled", krys::move(value));
  }

  dom::ExceptionOr<dom::USVString> HTMLButtonElement::FormAction() const noexcept
  {
    return Reflection::Reflect<dom::USVString>(*this, u8"formaction");
  }

  dom::ExceptionOr<void> HTMLButtonElement::FormAction(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectSetter<dom::USVString>(*this, u8"formaction", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::EncType() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"formenctype").value_or(u8"");
  }

  void HTMLButtonElement::EncType(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"formenctype", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::FormMethod() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"formmethod").value_or(u8"");
  }

  void HTMLButtonElement::FormMethod(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"formmethod", krys::move(value));
  }

  bool HTMLButtonElement::FormNoValidate() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"formnovalidate");
  }

  void HTMLButtonElement::FormNoValidate(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"formnovalidate", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::FormTarget() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"formtarget").value_or(u8"");
  }

  void HTMLButtonElement::FormTarget(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"formtarget", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLButtonElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLButtonElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::DOMString HTMLButtonElement::Value() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"value").value_or(u8"");
  }

  void HTMLButtonElement::Value(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"value", krys::move(value));
  }

#pragma endregion
}