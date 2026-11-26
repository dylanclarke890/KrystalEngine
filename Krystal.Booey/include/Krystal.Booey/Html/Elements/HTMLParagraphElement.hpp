#pragma once

#include "Krystal.Booey/Html/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#htmlparagraphelement
  class HTMLParagraphElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLParagraphElement);

  public:
    HTMLParagraphElement(Document &document) noexcept;

#pragma region HTMLParagraphElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLParagraphElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-p-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-p-align
    void Align(DOMString value) noexcept;

#pragma endregion

  protected:
    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLParagraphElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLParagraphElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
