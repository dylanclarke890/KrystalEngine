#include "Krystal.Booey/HTML/HTMLScriptElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

namespace krys::boo::html
{
  HTMLScriptElement::HTMLScriptElement(Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Script)
  {
  }

#pragma region HTMLScriptElement

  dom::DOMString HTMLScriptElement::Type() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLScriptElement::Type(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::ExceptionOr<USVString> HTMLScriptElement::Src() const noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"src");
  }

  dom::ExceptionOr<void> HTMLScriptElement::Src(USVString &&value) noexcept
  {
    return Attributes::Reflection::Reflect<USVString>(*this, u8"src", krys::move(value));
  }

  bool HTMLScriptElement::NoModule() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"nomodule");
  }

  void HTMLScriptElement::NoModule(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"nomodule", krys::move(value));
  }

  bool HTMLScriptElement::Async() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"async");
  }

  void HTMLScriptElement::Async(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"async", krys::move(value));
  }

  bool HTMLScriptElement::Defer() const noexcept
  {
    return Attributes::Reflection::Reflect<bool>(*this, u8"defer");
  }

  void HTMLScriptElement::Defer(bool value) noexcept
  {
    Attributes::Reflection::Reflect<bool>(*this, u8"defer", krys::move(value));
  }

  DOMTokenList &HTMLScriptElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking = ElementAlgorithms::CreateDOMTokenList(*this, u8"blocking");
    }

    return *_blocking;
  }

  Maybe<dom::DOMString> HTMLScriptElement::CrossOrigin() const noexcept
  {
    return Attributes::Reflection::Reflect<Maybe<dom::DOMString>>(*this, u8"type");
  }

  void HTMLScriptElement::CrossOrigin(Maybe<dom::DOMString> &&value) noexcept
  {
    Attributes::Reflection::Reflect<Maybe<dom::DOMString>>(*this, u8"type", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::ReferrerPolicy() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy").value_or(u8"");
  }

  void HTMLScriptElement::ReferrerPolicy(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::Integrity() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"integrity").value_or(u8"");
  }

  void HTMLScriptElement::Integrity(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"integrity", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::FetchPriority() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"fetchpriority").value_or(u8"");
  }

  void HTMLScriptElement::FetchPriority(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"fetchpriority", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::Text() const noexcept
  {
    // TODO(HTMLSCRIPTELEMENT): Implement the text attribute.
    return u8"";
  }

  void HTMLScriptElement::Text(dom::DOMString &&value) noexcept
  {
    // TODO(HTMLSCRIPTELEMENT): Implement the text attribute.
  }

#pragma endregion

#pragma region HTMLScriptElement Obsolete members

  dom::DOMString HTMLScriptElement::Charset() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLScriptElement::Charset(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"charset", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::Event() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"event").value_or(u8"");
  }

  void HTMLScriptElement::Event(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"event", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::HtmlFor() const noexcept
  {
    return Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"for").value_or(u8"");
  }

  void HTMLScriptElement::HtmlFor(dom::DOMString &&value) noexcept
  {
    Attributes::Reflection::Reflect<dom::DOMString>(*this, u8"for", krys::move(value));
  }

#pragma endregion
}