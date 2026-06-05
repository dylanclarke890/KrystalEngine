#include "Krystal.HTML/HTML/HTMLScriptElement.hpp"
#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/HTML/Attributes/Reflection.hpp"

namespace Krys::HTML
{
  HTMLScriptElement::HTMLScriptElement(Document &document) noexcept
      : HTMLElement(document, DOMInterface::Script)
  {
  }

#pragma region HTMLScriptElement

  DOMString HTMLScriptElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLScriptElement::Type(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"type", Krys::Move(value));
  }

  ExceptionOr<USVString> HTMLScriptElement::Src() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"src");
  }

  ExceptionOr<void> HTMLScriptElement::Src(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"src", Krys::Move(value));
  }

  bool HTMLScriptElement::NoModule() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"nomodule");
  }

  void HTMLScriptElement::NoModule(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"nomodule", Krys::Move(value));
  }

  bool HTMLScriptElement::Async() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"async");
  }

  void HTMLScriptElement::Async(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"async", Krys::Move(value));
  }

  bool HTMLScriptElement::Defer() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"defer");
  }

  void HTMLScriptElement::Defer(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"defer", Krys::Move(value));
  }

  DOMTokenList &HTMLScriptElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking = ElementAlgorithms::CreateDOMTokenList(*this, u8"blocking");
    }

    return *_blocking;
  }

  Maybe<DOMString> HTMLScriptElement::CrossOrigin() const noexcept
  {
    return Attributes::Reflection::Reflect<Maybe<DOMString>>(*this, u8"type");
  }

  void HTMLScriptElement::CrossOrigin(Maybe<DOMString> &&value) noexcept
  {
    Attributes::Reflection::Reflect<Maybe<DOMString>>(*this, u8"type", Krys::Move(value));
  }

  DOMString HTMLScriptElement::ReferrerPolicy() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLScriptElement::ReferrerPolicy(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"referrerpolicy", Krys::Move(value));
  }

  DOMString HTMLScriptElement::Integrity() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"integrity").value_or(u8"");
  }

  void HTMLScriptElement::Integrity(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"integrity", Krys::Move(value));
  }

  DOMString HTMLScriptElement::FetchPriority() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"fetchpriority").value_or(u8"");
  }

  void HTMLScriptElement::FetchPriority(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"fetchpriority", Krys::Move(value));
  }

  DOMString HTMLScriptElement::Text() const noexcept
  {
    // TODO(HTMLSCRIPTELEMENT): Implement the text attribute.
    return u8"";
  }

  void HTMLScriptElement::Text(DOMString &&value) noexcept
  {
    // TODO(HTMLSCRIPTELEMENT): Implement the text attribute.
  }

#pragma endregion

#pragma region HTMLScriptElement Obsolete members

  DOMString HTMLScriptElement::Charset() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLScriptElement::Charset(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"charset", Krys::Move(value));
  }

  DOMString HTMLScriptElement::Event() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"event").value_or(u8"");
  }

  void HTMLScriptElement::Event(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"event", Krys::Move(value));
  }

  DOMString HTMLScriptElement::HtmlFor() const noexcept
  {
    return Attributes::Reflection::Reflect<DOMString>(*this, u8"for").value_or(u8"");
  }

  void HTMLScriptElement::HtmlFor(DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<DOMString>(*this, u8"for", Krys::Move(value));
  }

#pragma endregion
}