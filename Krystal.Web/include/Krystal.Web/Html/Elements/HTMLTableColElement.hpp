#pragma once

#include "Krystal.Web/Html/Elements/HTMLTablePartElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/tables.html#htmltablecolelement
  class HTMLTableColElement : public HTMLTablePartElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableColElement);

  public:
    HTMLTableColElement(Document &document) noexcept;

#pragma region HTMLTableColElement - https://html.spec.whatwg.org/multipage/tables.html#htmltablecolelement

    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-colgroup-span
    KRYS_NODISCARD uint32 Span() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-colgroup-span
    void Span(uint32 value) noexcept;

#pragma endregion

#pragma region HTMLTableColElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLTableColElement-partial

    /// @see https://html.spec.whatwg.org/#dom-colgroup-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-align
    void Align(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-char
    KRYS_NODISCARD DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-char
    void Char(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-charoff
    KRYS_NODISCARD DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-charoff
    void CharOff(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-valign
    KRYS_NODISCARD DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-valign
    void VAlign(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-colgroup-width
    void Width(DOMString value) noexcept;

#pragma endregion

  public:
    KRYS_NODISCARD const css::MutableStyleProperties *
      AdditionalPresentationalHintStyle() const noexcept override;

  protected:
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLTableColElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableColElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
