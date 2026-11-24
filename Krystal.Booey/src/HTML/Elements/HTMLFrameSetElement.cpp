#include "Krystal.Booey/HTML/Elements/HTMLFrameSetElement.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo
{
  HTMLFrameSetElement::HTMLFrameSetElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::FrameSet)
  {
  }

  DOMString HTMLFrameSetElement::Cols() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"cols").value_or(u8"");
  }

  void HTMLFrameSetElement::Cols(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"cols", krys::move(value));
  }

  DOMString HTMLFrameSetElement::Rows() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"rows").value_or(u8"");
  }

  void HTMLFrameSetElement::Rows(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"rows", krys::move(value));
  }

  bool
    HTMLFrameSetElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                            const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null && (localName == u8"bordercolor"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLFrameSetElement::CollectPresentationalHintsForAttribute(
    const DOMStringAtom &localName, const DOMStringAtom &namespaceURI, DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
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
