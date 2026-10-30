#include "Krystal.Booey/HTML/HTMLUListElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Core/Text/ASCII.hpp"

namespace krys::boo::html
{
  HTMLUListElement::HTMLUListElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::UList)
  {
  }

#pragma region HTMLUListElement Obsolete members

  bool HTMLUListElement::Compact() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"compact");
  }

  void HTMLUListElement::Compact(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"compact", krys::move(value));
  }

  dom::DOMString HTMLUListElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLUListElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLUListElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                            dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"type"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLUListElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                                dom::DOMStringAtom namespaceURI,
                                                                dom::DOMStringView value,
                                                                css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"type")
    {
      {
        auto valueLowerCase = text::ToASCIILower(value);
        if (valueLowerCase == u8"disc")
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::ListStyleType, css::ValueId::Disc);
        }
        else if (valueLowerCase == u8"circle")
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::ListStyleType, css::ValueId::Circle);
        }
        else if (valueLowerCase == u8"round")
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::ListStyleType, css::ValueId::Round);
        }
        else if (valueLowerCase == u8"square")
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::ListStyleType, css::ValueId::Square);
        }
        else if (valueLowerCase == u8"none")
        {
          AddPropertyToPresentationalHintStyle(style, css::PropertyId::ListStyleType, css::ValueId::None);
        }
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
