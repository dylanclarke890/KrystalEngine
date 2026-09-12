#include "Krystal.Booey/HTML/HTMLFormElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLFormElement::HTMLFormElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Form)
  {
  }

#pragma region HTMLFormElement

  dom::DOMString HTMLFormElement::AcceptCharset() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"accept-charset").value_or(u8"");
  }

  void HTMLFormElement::AcceptCharset(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"accept-charset", krys::move(value));
  }

  dom::ExceptionOr<USVString> HTMLFormElement::Action() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"action");
  }

  dom::ExceptionOr<void> HTMLFormElement::Action(USVString &&value) noexcept
  {
    return Attributes::Reflection::ReflectSetter<USVString>(*this, u8"action", krys::move(value));
  }

  dom::DOMString HTMLFormElement::Autocomplete() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete").value_or(u8"");
  }

  void HTMLFormElement::Autocomplete(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"autocomplete", krys::move(value));
  }

  dom::DOMString HTMLFormElement::EncType() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"enctype").value_or(u8"");
  }

  void HTMLFormElement::EncType(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"enctype", krys::move(value));
  }

  dom::DOMString HTMLFormElement::Encoding() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"encoding").value_or(u8"");
  }

  void HTMLFormElement::Encoding(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"encoding", krys::move(value));
  }

  dom::DOMString HTMLFormElement::Method() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"method").value_or(u8"");
  }

  void HTMLFormElement::Method(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"method", krys::move(value));
  }

  dom::DOMString HTMLFormElement::Name() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLFormElement::Name(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"name", krys::move(value));
  }

  bool HTMLFormElement::NoValidate() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"novalidate");
  }

  void HTMLFormElement::NoValidate(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"novalidate", krys::move(value));
  }

  dom::DOMString HTMLFormElement::Target() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"target").value_or(u8"");
  }

  void HTMLFormElement::Target(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"target", krys::move(value));
  }

  dom::DOMString HTMLFormElement::Rel() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLFormElement::Rel(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"rel", krys::move(value));
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