#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmltablecellelement
  class HTMLTableCellElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableCellElement);

  public:
    HTMLTableCellElement(Document &document) noexcept;

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
    KRYS_NODISCARD DOMString Headers() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-headers
    void Headers(DOMString &&value) noexcept;

    // TODO(HTMLTableCellElement, HTML): Implement CellIndex().
    /// @see https://html.spec.whatwg.org/#dom-tdth-cellindex
    //KRYS_NODISCARD int32 CellIndex() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-scope
    KRYS_NODISCARD DOMString Scope() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-scope
    void Scope(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-abbr
    KRYS_NODISCARD DOMString Abbr() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-abbr
    void Abbr(DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLTableCellElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableCellElement-partial

    /// @see https://html.spec.whatwg.org/#dom-tdth-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-align
    void Align(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-axis
    KRYS_NODISCARD DOMString Axis() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-axis
    void Axis(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-height
    KRYS_NODISCARD DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-height
    void Height(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-width
    void Width(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-char
    KRYS_NODISCARD DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-char
    void Char(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-charoff
    KRYS_NODISCARD DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-charoff
    void CharOff(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-nowrap
    KRYS_NODISCARD bool NoWrap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-nowrap
    void NoWrap(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-valign
    KRYS_NODISCARD DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-valign
    void VAlign(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-bgcolor
    KRYS_NODISCARD DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-bgcolor
    void BgColor(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLTableCellElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableCellElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();