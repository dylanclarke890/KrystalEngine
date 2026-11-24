#include "Krystal.Booey/HTML/Elements/HTMLPlugInElement.hpp"
#include "Krystal.Booey/HTML/Internals/PresentationalHints.hpp"

namespace krys::boo
{
  HTMLPlugInElement::HTMLPlugInElement(Document &document, html::HTMLElementInterface interface) noexcept
      : HTMLElement(document, interface)
  {
  }

  bool HTMLPlugInElement::HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                             const DOMStringAtom &namespaceURI) const noexcept
  {
    if (namespaceURI == null
        && (localName == u8"width" || localName == u8"height" || localName == u8"vspace"
            || localName == u8"hspace" || localName == u8"align"))
    {
      return true;
    }
    return HTMLElement::HasPresentationalHintsForAttribute(localName, namespaceURI);
  }

  void HTMLPlugInElement::CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
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
    if (localName == u8"width")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Width, value);
      return;
    }
    if (localName == u8"height")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::Height, value);
      return;
    }
    if (localName == u8"vspace")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::MarginTop, value);
      AddHTMLLengthToStyle(style, css::PropertyId::MarginBottom, value);
      return;
    }
    if (localName == u8"hspace")
    {
      AddHTMLLengthToStyle(style, css::PropertyId::MarginLeft, value);
      AddHTMLLengthToStyle(style, css::PropertyId::MarginRight, value);
      return;
    }
    if (localName == u8"align")
    {
      ApplyAlignmentAttributeToStyle(value, style);
      return;
    }
    HTMLElement::CollectPresentationalHintsForAttribute(localName, namespaceURI, value, style);
  }
}
