#include "Krystal.Booey/HTML/HTMLLIElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Numbers.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "string"

namespace krys::boo::html
{
  HTMLLIElement::HTMLLIElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::LI)
  {
  }

#pragma region HTMLLIElement

  int32 HTMLLIElement::Value() const noexcept
  {
    return Reflection::Reflect<int32>(*this, u8"value");
  }

  void HTMLLIElement::Value(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"value", krys::move(value));
  }

#pragma endregion

#pragma region HTMLLIElement Obsolete members

  dom::DOMString HTMLLIElement::Type() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLLIElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLLIElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                         dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"type" || localName == u8"value"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLLIElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
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
      if (value == u8"a")
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::ListStyleType, css::ValueId::LowerAlpha);
      }
      else if (value == u8"A")
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::ListStyleType, css::ValueId::UpperAlpha);
      }
      else if (value == u8"i")
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::ListStyleType, css::ValueId::LowerRoman);
      }
      else if (value == u8"I")
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::ListStyleType, css::ValueId::UpperRoman);
      }
      else if (value == u8"1")
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::ListStyleType, css::ValueId::Decimal);
      }
      else
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
    if (localName == u8"value")
    {
      if (auto parsedValue = Numbers::ParseInteger(value);
          parsedValue.Success() && parsedValue.Value >= std::numeric_limits<int32>::min()
          && parsedValue.Value <= std::numeric_limits<int32>::max())
      {
        auto counter = css::CSSOMString(u8"list-item ");
        counter +=
          css::CSSOMString(reinterpret_cast<const char8 *>(std::to_string(parsedValue.Value).c_str()));
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::CounterSet, counter);
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
