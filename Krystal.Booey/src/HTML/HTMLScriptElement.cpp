#include "Krystal.Booey/HTML/HTMLScriptElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"

#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  HTMLScriptElement::HTMLScriptElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Script)
  {
  }

#pragma region HTMLScriptElement

  dom::DOMString HTMLScriptElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLScriptElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::USVString HTMLScriptElement::Src() const noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"src");
  }

  void HTMLScriptElement::Src(dom::USVString &&value) noexcept
  {
    return Reflection::ReflectURL<dom::USVString>(*this, u8"src", krys::move(value));
  }

  bool HTMLScriptElement::NoModule() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"nomodule");
  }

  void HTMLScriptElement::NoModule(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"nomodule", krys::move(value));
  }

  bool HTMLScriptElement::Async() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"async");
  }

  void HTMLScriptElement::Async(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"async", krys::move(value));
  }

  bool HTMLScriptElement::Defer() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"defer");
  }

  void HTMLScriptElement::Defer(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"defer", krys::move(value));
  }

  dom::DOMTokenList &HTMLScriptElement::Blocking() noexcept
  {
    if (!_blocking)
    {
      _blocking = dom::ElementAlgorithms::CreateDOMTokenList(
        *this, u8"blocking", [](dom::Document &, dom::DOMStringView token)
        { return text::IsASCIICaselessEqual(token, utf8_stringview(u8"render")); });
    }

    return *_blocking;
  }

  void HTMLScriptElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI == dom::DOMStringAtom::Null() && localName == u8"blocking" && _blocking)
    {
      _blocking->AssociatedAttributeValueChanged();
    }
  }

  Maybe<dom::DOMString> HTMLScriptElement::CrossOrigin() const noexcept
  {
    return EnumeratedAttribute<"crossorigin", HTMLElement>::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(
      Reflection::Reflect<dom::DOMString>(*this, u8"crossorigin"));
  }

  void HTMLScriptElement::CrossOrigin(Maybe<dom::DOMString> &&value) noexcept
  {
    Reflection::Reflect<Maybe<dom::DOMString>>(*this, u8"crossorigin", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::ReferrerPolicy() const noexcept
  {
    return EnumeratedAttribute<"referrerpolicy", HTMLElement>::ResolveCanonicalKeyword<dom::DOMString>(
      Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy"));
  }

  void HTMLScriptElement::ReferrerPolicy(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::Integrity() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"integrity").value_or(u8"");
  }

  void HTMLScriptElement::Integrity(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"integrity", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::FetchPriority() const noexcept
  {
    return EnumeratedAttribute<"fetchpriority", HTMLElement>::ResolveCanonicalKeyword<dom::DOMString>(
      Reflection::Reflect<dom::DOMString>(*this, u8"fetchpriority"));
  }

  void HTMLScriptElement::FetchPriority(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"fetchpriority", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::Text() const noexcept
  {
    // FIXME: Implement the text attribute.
    krys_not_implemented();
  }

  void HTMLScriptElement::Text(dom::DOMString &&value) noexcept
  {
    (void)value;
    krys_not_implemented();
    // FIXME: Implement the text attribute.
  }

#pragma endregion

#pragma region HTMLScriptElement Obsolete members

  dom::DOMString HTMLScriptElement::Charset() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLScriptElement::Charset(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"charset", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::Event() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"event").value_or(u8"");
  }

  void HTMLScriptElement::Event(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"event", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::HtmlFor() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"for").value_or(u8"");
  }

  void HTMLScriptElement::HtmlFor(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"for", krys::move(value));
  }

#pragma endregion
}