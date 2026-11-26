#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/tables.html#htmltablecaptionelement
  class HTMLTableCaptionElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableCaptionElement);

  public:
    HTMLTableCaptionElement(Document &document) noexcept;

#pragma region HTMLTableCaptionElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLTableCaptionElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-caption-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-caption-align
    void Align(DOMString value) noexcept;

#pragma endregion

  protected:
    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLTableCaptionElement)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return Is<krys::web::HTMLElement>(target)
           && Downcast<krys::web::HTMLElement>(target).IsHTMLTableCaptionElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableCaptionElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
