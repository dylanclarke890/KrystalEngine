#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#htmltablecaptionelement
  class HTMLTableCaptionElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableCaptionElement);

  public:
    HTMLTableCaptionElement(Document &document) noexcept;

#pragma region HTMLTableCaptionElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableCaptionElement-partial

    /// @see https://html.spec.whatwg.org/#dom-caption-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-caption-align
    void Align(DOMString value) noexcept;

#pragma endregion

  protected:
    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLTableCaptionElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return Is<krys::boo::HTMLElement>(target)
           && Downcast<krys::boo::HTMLElement>(target).IsHTMLTableCaptionElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableCaptionElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
