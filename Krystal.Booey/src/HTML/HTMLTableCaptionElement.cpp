#include "Krystal.Booey/HTML/HTMLTableCaptionElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo::html
{
  HTMLTableCaptionElement::HTMLTableCaptionElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::TableCaption)
  {
  }

#pragma region HTMLTableCaptionElement Obsolete members

  dom::DOMString HTMLTableCaptionElement::Align() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableCaptionElement::Align(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"align", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  void HTMLTableCaptionElement::CollectPresentationalHintsForAttribute(
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
      if (!value.empty())
      {
        AddPropertyToPresentationalHintStyle(style, css::PropertyId::CaptionSide, ToCSSOMString(value));
      }
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }

#pragma endregion
}
