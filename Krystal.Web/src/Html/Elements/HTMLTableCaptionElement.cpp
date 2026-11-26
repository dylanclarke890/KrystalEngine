#include "Krystal.Web/Html/Elements/HTMLTableCaptionElement.hpp"
#include "Krystal.Web/Html/Attributes/ReflectAttribute.hpp"
#include "Krystal.Web/Html/Internals/PresentationalHints.hpp"

namespace krys::web
{
  HTMLTableCaptionElement::HTMLTableCaptionElement(Document &document) noexcept
      : HTMLElement(document, html::HTMLElementInterface::TableCaption)
  {
  }

#pragma region HTMLTableCaptionElement Obsolete members

  DOMString HTMLTableCaptionElement::Align() const noexcept
  {
    return html::Reflect<DOMString>(*this, u8"align").value_or(u8"");
  }

  void HTMLTableCaptionElement::Align(DOMString value) noexcept
  {
    html::Reflect<DOMString>(*this, u8"align", krys::move(value));
  }

#pragma endregion

#pragma region Presentational hints

  void HTMLTableCaptionElement::CollectPresentationalHintsForAttribute(
    const DOMStringAtom &localName, const DOMStringAtom &namespaceURI, DOMStringView value,
    css::MutableStyleProperties &style) noexcept
  {
    using namespace html::presentational_hints;
    if (namespaceURI != null)
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
