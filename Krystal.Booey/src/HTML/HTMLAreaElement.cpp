#include "Krystal.Booey/HTML/HTMLAreaElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLAreaElement::HTMLAreaElement(Document &document) noexcept : HTMLElement(document, HTMLElementInterface::Area)
  {
  }

#pragma region HTMLAreaElement

  dom::DOMString HTMLAreaElement::Alt() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"alt").value_or(u8"");
  }

  void HTMLAreaElement::Alt(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"alt", krys::move(value));
  }

  dom::DOMString HTMLAreaElement::Coords() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"coords").value_or(u8"");
  }

  void HTMLAreaElement::Coords(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"coords", krys::move(value));
  }

  dom::DOMString HTMLAreaElement::Shape() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"shape").value_or(u8"");
  }

  void HTMLAreaElement::Shape(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"shape", krys::move(value));
  }

  dom::DOMString HTMLAreaElement::Download() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"download").value_or(u8"");
  }

  void HTMLAreaElement::Download(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"download", krys::move(value));
  }

  dom::ExceptionOr<USVString> HTMLAreaElement::Ping() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"ping");
  }

  dom::ExceptionOr<void> HTMLAreaElement::Ping(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"ping", krys::move(value));
  }

  dom::DOMString HTMLAreaElement::Rel() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLAreaElement::Rel(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"rel", krys::move(value));
  }

  DOMTokenList &HTMLAreaElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = ElementAlgorithms::CreateDOMTokenList(*this, u8"rel");
    }

    return *_relList;
  }

  dom::DOMString HTMLAreaElement::ReferrerPolicy() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLAreaElement::ReferrerPolicy(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

#pragma endregion

#pragma region HTMLAreaElement Obsolete members

  bool HTMLAreaElement::NoHref() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"nohref");
  }

  void HTMLAreaElement::NoHref(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"nohref", krys::move(value));
  }

#pragma endregion
}
