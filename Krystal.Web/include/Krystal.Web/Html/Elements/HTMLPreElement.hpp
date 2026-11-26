#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#htmlpreelement
  class HTMLPreElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLPreElement);

  public:
    HTMLPreElement(Document &document) noexcept;

#pragma region HTMLPreElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLPreElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-pre-width
    KRYS_NODISCARD int32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-pre-width
    void Width(int32 value) noexcept;

#pragma endregion

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLPreElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLPreElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
