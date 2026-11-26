#include "Krystal.Booey/Html/Elements/HTMLUListElement.hpp"
#include "Krystal.Booey/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/Html/Internals/PresentationalHints.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"

namespace krys::boo
{
  HTMLUListElement::HTMLUListElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::UList)
  {
  }

#pragma region HTMLUListElement Obsolete members

  bool HTMLUListElement::Compact() const noexcept
  {
    return html::Reflect<bool>(*this, u8"compact");
  }

  void HTMLUListElement::Compact(bool value) noexcept
  {
    html::Reflect<bool>(*this, u8"compact", krys::move(value));
  }

  DOMString HTMLUListElement::Type() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLUListElement::Type(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"type", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLUListElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                            const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null && (localName == u8"type"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLUListElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
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
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
