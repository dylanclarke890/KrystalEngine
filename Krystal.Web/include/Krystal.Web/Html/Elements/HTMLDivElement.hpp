#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#htmldivelement
  class HTMLDivElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDivElement);

  public:
    HTMLDivElement(Document &document) noexcept;

#pragma region HTMLDivElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLDivElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-div-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-div-align
    void Align(DOMString value) noexcept;

#pragma endregion

  protected:
    HTMLDivElement(Document &document, dom::NodeFlags nodeFlags) noexcept;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLDivElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLDivElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
