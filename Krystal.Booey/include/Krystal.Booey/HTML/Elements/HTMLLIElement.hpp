#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#htmllielement
  class HTMLLIElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLIElement);

  public:
    HTMLLIElement(Document &document) noexcept;

#pragma region HTMLLIElement - https://html.spec.whatwg.org/multipage/grouping-content.html#htmllielement

    /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#dom-li-value
    KRYS_NODISCARD int32 Value() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/grouping-content.html#dom-li-value
    void Value(int32 value) noexcept;

#pragma endregion

#pragma region HTMLLIElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLLIElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-li-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-li-type
    void Type(DOMString value) noexcept;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLLIElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLLIElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
