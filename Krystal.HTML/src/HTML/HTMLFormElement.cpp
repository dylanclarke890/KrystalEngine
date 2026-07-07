#include "Krystal.HTML/HTML/HTMLFormElement.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLFormElement::HTMLFormElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Form)
  {
  }

#pragma region HTMLFormElement

  DOMString HTMLFormElement::AcceptCharset() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"accept-charset").value_or(u8"");
  }

  void HTMLFormElement::AcceptCharset(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"accept-charset", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLFormElement::Action() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"action");
  }

  ExceptionOr<void> HTMLFormElement::Action(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectSetter<USVString>(*this, u8"action", Krys::Move(value));
  }

  DOMString HTMLFormElement::Autocomplete() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLFormElement::Autocomplete(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"autocomplete", Krys::Move(value));
  }

  DOMString HTMLFormElement::EncType() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"enctype").value_or(u8"");
  }

  void HTMLFormElement::EncType(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"enctype", Krys::Move(value));
  }

  DOMString HTMLFormElement::Encoding() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"encoding").value_or(u8"");
  }

  void HTMLFormElement::Encoding(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"encoding", Krys::Move(value));
  }

  DOMString HTMLFormElement::Method() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"method").value_or(u8"");
  }

  void HTMLFormElement::Method(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"method", Krys::Move(value));
  }

  DOMString HTMLFormElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLFormElement::Name(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"name", Krys::Move(value));
  }

  bool HTMLFormElement::NoValidate() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"novalidate");
  }

  void HTMLFormElement::NoValidate(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"novalidate", Krys::Move(value));
  }

  DOMString HTMLFormElement::Target() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLFormElement::Target(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"target", Krys::Move(value));
  }

  DOMString HTMLFormElement::Rel() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLFormElement::Rel(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"rel", Krys::Move(value));
  }

  DOMTokenList &HTMLFormElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = ElementAlgorithms::CreateDOMTokenList(*this, u8"rel");
    }

    return *_relList;
  }

#pragma endregion
}