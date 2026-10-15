#include "Krystal.Booey/HTML/HTMLOListElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo::html
{
  HTMLOListElement::HTMLOListElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::OList)
  {
  }

#pragma region HTMLOListElement

  bool HTMLOListElement::Reversed() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"reversed");
  }

  void HTMLOListElement::Reversed(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"reversed", krys::move(value));
  }

  int32 HTMLOListElement::Start() const noexcept
  {
    return Reflection::Reflect<int32, ReflectDefault<int32>(1)>(*this, u8"start");
  }

  void HTMLOListElement::Start(int32 value) noexcept
  {
    Reflection::Reflect<int32>(*this, u8"start", krys::move(value));
  }

  dom::DOMString HTMLOListElement::Type() const noexcept
  {
    // TODO(HTMLOLISTLEMENT, HTML): Type is an enumerated attribute.
    return Reflection::Reflect<dom::DOMString>(*this, u8"type").value_or(u8"");
  }

  void HTMLOListElement::Type(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"type", krys::move(value));
  }

#pragma endregion

#pragma region HTMLOListElement Obsolete members

  bool HTMLOListElement::Compact() const noexcept
  {
    return Reflection::Reflect<bool>(*this, u8"compact");
  }

  void HTMLOListElement::Compact(bool value) noexcept
  {
    Reflection::Reflect<bool>(*this, u8"compact", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  bool HTMLOListElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                            dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"type"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLOListElement::CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName,
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
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
