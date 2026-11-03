#include "Krystal.Booey/HTML/Elements/HTMLScriptElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"

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
    return Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLScriptElement::Type(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

  dom::USVString HTMLScriptElement::Src() const noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"src");
  }

  void HTMLScriptElement::Src(dom::USVString value) noexcept
  {
    return ReflectURL<dom::USVString>(*this, u8"src", krys::move(value));
  }

  bool HTMLScriptElement::NoModule() const noexcept
  {
    return Reflect<bool>(*this, u8"nomodule");
  }

  void HTMLScriptElement::NoModule(bool value) noexcept
  {
    Reflect<bool>(*this, u8"nomodule", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-script-async
  bool HTMLScriptElement::Async() const noexcept
  {
    // 1. If this's force async is true, then return true.
    // 2. If this's async content attribute is present, then return true.
    // 3. Return false.
    return _forceAsync || Reflect<bool>(*this, u8"async");
  }

  // https://html.spec.whatwg.org/#dom-script-async
  void HTMLScriptElement::Async(bool value) noexcept
  {
    // 1. Set this's force async to false.
    _forceAsync = false;

    // 2. If the given value is true, then set this's async content attribute to the empty string.
    // 3. Otherwise, remove this's async content attribute.
    Reflect<bool>(*this, u8"async", krys::move(value));
  }

  bool HTMLScriptElement::Defer() const noexcept
  {
    return Reflect<bool>(*this, u8"defer");
  }

  void HTMLScriptElement::Defer(bool value) noexcept
  {
    Reflect<bool>(*this, u8"defer", krys::move(value));
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
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      return;
    }
    if (localName == u8"blocking" && _blocking)
    {
      _blocking->AssociatedAttributeValueChanged();
    }
    // When an async attribute is added to a script element el, the user agent must set el's force async to
    // false.
    else if (localName == u8"async" && value && !oldValue)
    {
      _forceAsync = false;
    }
  }

  // https://html.spec.whatwg.org/#the-script-element:concept-node-clone-ext
  void HTMLScriptElement::OnClone(dom::Node &copy, bool subtree) noexcept
  {
    HTMLElement::OnClone(copy, subtree);

    // The cloning steps for script elements given node, copy, and subtree are to set copy's already started
    // to node's already started.
    Downcast<HTMLScriptElement>(copy)._alreadyStarted = _alreadyStarted;
  }

  Maybe<dom::DOMString> HTMLScriptElement::CrossOrigin() const noexcept
  {
    return EnumeratedAttribute<"crossorigin", HTMLElement>::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(
      Reflect<dom::DOMString>(*this, u8"crossorigin"));
  }

  void HTMLScriptElement::CrossOrigin(Maybe<dom::DOMString> value) noexcept
  {
    Reflect<Maybe<dom::DOMString>>(*this, u8"crossorigin", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::ReferrerPolicy() const noexcept
  {
    return EnumeratedAttribute<"referrerpolicy", HTMLElement>::ResolveCanonicalKeyword<dom::DOMString>(
      Reflect<dom::DOMString>(*this, u8"referrerpolicy"));
  }

  void HTMLScriptElement::ReferrerPolicy(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"referrerpolicy", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::Integrity() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"integrity").value_or(u8"");
  }

  void HTMLScriptElement::Integrity(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"integrity", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::FetchPriority() const noexcept
  {
    return EnumeratedAttribute<"fetchpriority", HTMLElement>::ResolveCanonicalKeyword<dom::DOMString>(
      Reflect<dom::DOMString>(*this, u8"fetchpriority"));
  }

  void HTMLScriptElement::FetchPriority(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"fetchpriority", krys::move(value));
  }

  // https://html.spec.whatwg.org/#dom-script-text
  dom::DOMString HTMLScriptElement::Text() const noexcept
  {
    // The text getter steps are to return this's child text content.
    return dom::TextAlgorithms::ChildTextContent(*this);
  }

  // https://html.spec.whatwg.org/#dom-script-text
  void HTMLScriptElement::Text(dom::DOMString value) noexcept
  {
    // 1. (Skipped: there are no Trusted Types.)
    // 2. (Skipped: there are no Trusted Types.)
    // 3. String replace all with compliantScript within this.
    (void)dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

  // https://html.spec.whatwg.org/#dom-script-supports
  bool HTMLScriptElement::Supports(dom::DOMStringView type) noexcept
  {
    // 1. If type is "classic", then return true.
    // 2. If type is "module", then return true.
    // 3. If type is "importmap", then return true.
    // 4. If type is "speculationrules", then return true.
    // 5. Return false.
    return type == u8"classic" || type == u8"module" || type == u8"importmap" || type == u8"speculationrules";
  }

#pragma endregion

#pragma region HTMLScriptElement Obsolete members

  dom::DOMString HTMLScriptElement::Charset() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"charset").value_or(u8"");
  }

  void HTMLScriptElement::Charset(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"charset", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::Event() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"event").value_or(u8"");
  }

  void HTMLScriptElement::Event(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"event", krys::move(value));
  }

  dom::DOMString HTMLScriptElement::HtmlFor() const noexcept
  {
    return Reflect<dom::DOMString>(*this, u8"for").value_or(u8"");
  }

  void HTMLScriptElement::HtmlFor(dom::DOMString value) noexcept
  {
    Reflect<dom::DOMString>(*this, u8"for", krys::move(value));
  }

#pragma endregion
}