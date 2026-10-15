#include "Krystal.Booey/HTML/HTMLParagraphElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo::html
{
  HTMLParagraphElement::HTMLParagraphElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::Heading)
  {
  }

#pragma region HTMLHeadingElement Obsolete members

  dom::DOMString HTMLParagraphElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLParagraphElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  void HTMLParagraphElement::CollectPresentationalHintsForAttribute(
    dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI, dom::DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"align")
    {
      if (EqualLettersIgnoringASCIICase(value, u8"middle")
          || EqualLettersIgnoringASCIICase(value, u8"center"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::WebkitCenter);
      }
      else if (EqualLettersIgnoringASCIICase(value, u8"left"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::WebkitLeft);
      }
      else if (EqualLettersIgnoringASCIICase(value, u8"right"))
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, css::ValueId::WebkitRight);
      }
      else
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::TextAlign, ToCSSOMString(value));
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
