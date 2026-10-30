#pragma once

#include "Krystal.Booey/HTML/HTMLTablePartElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmltablecellelement
  class HTMLTableCellElement : public HTMLTablePartElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableCellElement);

  public:
    HTMLTableCellElement(dom::Document &document) noexcept;

#pragma region HTMLTableCellElement - https://html.spec.whatwg.org/#htmltablecellelement

    /// @see https://html.spec.whatwg.org/#dom-tdth-colspan
    KRYS_NODISCARD uint32 ColSpan() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-colspan
    void ColSpan(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-rowspan
    KRYS_NODISCARD uint32 RowSpan() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-rowspan
    void RowSpan(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-headers
    KRYS_NODISCARD dom::DOMString Headers() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-headers
    void Headers(dom::DOMString &&value) noexcept;

    /// @brief The cell's index in its row's cells, or -1 if its parent isn't a row.
    /// @see https://html.spec.whatwg.org/#dom-tdth-cellindex
    KRYS_NODISCARD int32 CellIndex() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-scope
    KRYS_NODISCARD dom::DOMString Scope() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-scope
    void Scope(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-abbr
    KRYS_NODISCARD dom::DOMString Abbr() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-abbr
    void Abbr(dom::DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLTableCellElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableCellElement-partial

    /// @see https://html.spec.whatwg.org/#dom-tdth-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-axis
    KRYS_NODISCARD dom::DOMString Axis() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-axis
    void Axis(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-height
    KRYS_NODISCARD dom::DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-height
    void Height(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-width
    KRYS_NODISCARD dom::DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-width
    void Width(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-char
    KRYS_NODISCARD dom::DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-char
    void Char(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-charoff
    KRYS_NODISCARD dom::DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-charoff
    void CharOff(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-nowrap
    KRYS_NODISCARD bool NoWrap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-nowrap
    void NoWrap(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-valign
    KRYS_NODISCARD dom::DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-valign
    void VAlign(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-bgcolor
    KRYS_NODISCARD dom::DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-bgcolor
    void BgColor(dom::DOMString &&value) noexcept;

#pragma endregion

  public:
    KRYS_NODISCARD const css::MutableStyleProperties *
      AdditionalPresentationalHintStyle() const noexcept override;

  protected:
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTableCellElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableCellElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();