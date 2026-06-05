#include "Krystal.HTML/HTML/HTMLAreaElement.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLAreaElement::HTMLAreaElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Area)
  {
  }

#pragma region HTMLAreaElement

  DOMString HTMLAreaElement::Alt() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"alt").value_or(u8"");
  }

  void HTMLAreaElement::Alt(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"alt", Krys::Move(value));
  }

  DOMString HTMLAreaElement::Coords() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"coords").value_or(u8"");
  }

  void HTMLAreaElement::Coords(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"coords", Krys::Move(value));
  }

  DOMString HTMLAreaElement::Shape() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"shape").value_or(u8"");
  }

  void HTMLAreaElement::Shape(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"shape", Krys::Move(value));
  }

  DOMString HTMLAreaElement::Download() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"download").value_or(u8"");
  }

  void HTMLAreaElement::Download(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"download", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLAreaElement::Ping() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"ping");
  }

  ExceptionOr<void> HTMLAreaElement::Ping(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"ping", Krys::Move(value));
  }

  DOMString HTMLAreaElement::Rel() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"rel").value_or(u8"");
  }

  void HTMLAreaElement::Rel(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"rel", Krys::Move(value));
  }

  DOMTokenList &HTMLAreaElement::RelList() noexcept
  {
    if (!_relList)
    {
      _relList = ElementAlgorithms::CreateDOMTokenList(*this, u8"rel");
    }

    return *_relList;
  }

  DOMString HTMLAreaElement::ReferrerPolicy() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLAreaElement::ReferrerPolicy(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"referrerpolicy", Krys::Move(value));
  }

#pragma endregion

#pragma region HTMLAreaElement Obsolete members

  bool HTMLAreaElement::NoHref() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"nohref");
  }

  void HTMLAreaElement::NoHref(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"nohref", Krys::Move(value));
  }

#pragma endregion
}
