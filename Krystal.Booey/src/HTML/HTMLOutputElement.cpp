#include "Krystal.Booey/HTML/HTMLOutputElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"

namespace krys::boo::html
{
  HTMLOutputElement::HTMLOutputElement(dom::Document &document) noexcept
      : HTMLFormControlElement(document, HTMLElementInterface::Output)
  {
  }

  HTMLOutputElement::~HTMLOutputElement() noexcept = default;

  void HTMLOutputElement::Reset() noexcept
  {
    SetInteractedWithSinceLastFormSubmitEvent(false);
    (void)dom::NodeAlgorithms::StringReplaceAll(DefaultValue(), *this);
    _defaultValueOverride = std::nullopt;
  }

#pragma region HTMLOutputElement

  dom::DOMTokenList &HTMLOutputElement::HtmlFor() noexcept
  {
    if (!_htmlFor)
    {
      _htmlFor = dom::ElementAlgorithms::CreateDOMTokenList(*this, u8"for");
    }

    return *_htmlFor;
  }

  void HTMLOutputElement::OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                                             Maybe<dom::DOMStringView> value,
                                             dom::DOMStringAtom namespaceURI) noexcept
  {
    HTMLFormControlElement::OnAttributeChanged(localName, oldValue, value, namespaceURI);
    if (namespaceURI == dom::DOMStringAtom::Null() && localName == u8"for" && _htmlFor)
    {
      _htmlFor->AssociatedAttributeValueChanged();
    }
  }

  dom::DOMString HTMLOutputElement::Name() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"name").value_or(u8"");
  }

  void HTMLOutputElement::Name(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect(*this, u8"name", krys::move(value));
  }

  dom::DOMString HTMLOutputElement::Value() const noexcept
  {
    return TextContent().value_or(dom::DOMString());
  }

  void HTMLOutputElement::Value(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    _defaultValueOverride = DefaultValue();
    (void)dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

  dom::DOMString HTMLOutputElement::DefaultValue() const noexcept
  {
    return _defaultValueOverride ? *_defaultValueOverride : TextContent().value_or(dom::DOMString());
  }

  void HTMLOutputElement::DefaultValue(dom::DOMString &&value) noexcept
  {
    CEReactionsScope ceReactions;
    if (!_defaultValueOverride)
    {
      (void)dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
    }
    else
    {
      _defaultValueOverride = krys::move(value);
    }
  }

#pragma endregion
}
