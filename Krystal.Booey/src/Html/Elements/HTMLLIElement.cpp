#include "Krystal.Booey/Html/Elements/HTMLLIElement.hpp"
#include "Krystal.Booey/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/Html/Internals/PresentationalHints.hpp"
#include "Krystal.Booey/Html/MicroParsers/Numbers.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "string"

namespace krys::boo
{
  HTMLLIElement::HTMLLIElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::LI)
  {
  }

#pragma region HTMLLIElement

  int32 HTMLLIElement::Value() const noexcept
  {
    return html::Reflect<int32>(*this, u8"value");
  }

  void HTMLLIElement::Value(int32 value) noexcept
  {
    html::Reflect<int32>(*this, u8"value", krys::move(value));
  }

#pragma endregion

#pragma region HTMLLIElement Obsolete members

  DOMString HTMLLIElement::Type() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLLIElement::Type(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"type", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLLIElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                         const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null && (localName == u8"type" || localName == u8"value"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLLIElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                             const DOMStringAtom &namespaceURI,
                                                             DOMStringView value,
                                                             css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
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
      if (auto parsedValue = html::Numbers::ParseInteger(value);
          parsedValue.Success() && parsedValue.Value >= std::numeric_limits<int32>::min()
          && parsedValue.Value <= std::numeric_limits<int32>::max())
      {
        auto counter = CSSOMString(u8"list-item ");
        counter += CSSOMString(reinterpret_cast<const char8 *>(std::to_string(parsedValue.Value).c_str()));
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::CounterSet, counter);
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
