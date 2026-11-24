#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#htmlulistelement
  class HTMLUListElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLUListElement);

  public:
    HTMLUListElement(Document &document) noexcept;

#pragma region HTMLUListElement Obsolete members - https://html.spec.whatwg.org/#HTMLUListElement-partial

    /// @see https://html.spec.whatwg.org/#dom-ul-compact
    KRYS_NODISCARD bool Compact() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ul-compact
    void Compact(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ul-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ul-type
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLUListElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLUListElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
