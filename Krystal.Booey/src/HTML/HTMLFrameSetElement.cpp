#include "Krystal.Booey/HTML/HTMLFrameSetElement.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo::html
{
  HTMLFrameSetElement::HTMLFrameSetElement(dom::Document &document) noexcept
      : HTMLElement(document, HTMLElementInterface::FrameSet)
  {
  }

  dom::DOMString HTMLFrameSetElement::Cols() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"cols").value_or(u8"");
  }

  void HTMLFrameSetElement::Cols(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"cols", krys::move(value));
  }

  dom::DOMString HTMLFrameSetElement::Rows() const noexcept
  {
    return Reflection::Reflect<dom::DOMString>(*this, u8"rows").value_or(u8"");
  }

  void HTMLFrameSetElement::Rows(dom::DOMString &&value) noexcept
  {
    Reflection::Reflect<dom::DOMString>(*this, u8"rows", krys::move(value));
  }

  bool HTMLFrameSetElement::HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                                               dom::DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == dom::DOMStringAtom::Null() && (localName == u8"bordercolor"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLFrameSetElement::CollectPresentationalHintsForAttribute(
    dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI, dom::DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace presentational_hints;
    if (namespaceURI != dom::DOMStringAtom::Null())
    {
      HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
      return;
    }
    if (localName == u8"bordercolor")
    {
      AddHTMLColorToStyle(style, css::PropertyId::BorderColor, value);
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }
}
