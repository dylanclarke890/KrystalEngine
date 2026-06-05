#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmltableelement
  class HTMLTableElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableElement);

  public:
    HTMLTableElement(Document &document) noexcept;

#pragma region HTMLTableElement - https://html.spec.whatwg.org/#htmltableelement

    // TODO(HTMLTableElement, HTML): Implement Caption(), CreateCaption(), DeleteCaption(), THead(),
    // CreateTHead(), DeleteTHead(),
    // [CEReactions] attribute HTMLTableCaptionElement? caption;
    // HTMLTableCaptionElement createCaption();
    // [CEReactions] undefined deleteCaption();
    //
    // [CEReactions] attribute HTMLTableSectionElement? tHead;
    // HTMLTableSectionElement createTHead();
    // [CEReactions] undefined deleteTHead();
    //
    // [CEReactions] attribute HTMLTableSectionElement? tFoot;
    // HTMLTableSectionElement createTFoot();
    // [CEReactions] undefined deleteTFoot();
    //
    // [SameObject] readonly attribute HTMLCollection tBodies;
    // HTMLTableSectionElement createTBody();
    //
    // [SameObject] readonly attribute HTMLCollection rows;
    // HTMLTableRowElement insertRow(optional long index = -1);
    // [CEReactions] undefined deleteRow(long index);

#pragma endregion

#pragma region HTMLTableElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableElement-partial

    /// @see https://html.spec.whatwg.org/#dom-table-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-align
    void Align(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-border
    KRYS_NODISCARD DOMString Border() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-border
    void Border(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-frame
    KRYS_NODISCARD DOMString Frame() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-frame
    void Frame(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-rules
    KRYS_NODISCARD DOMString Rules() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-rules
    void Rules(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-summary
    KRYS_NODISCARD DOMString Summary() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-summary
    void Summary(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-width
    void Width(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-bgcolor
    KRYS_NODISCARD DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-bgcolor
    void BgColor(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-cellpadding
    KRYS_NODISCARD DOMString CellPadding() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-cellpadding
    void CellPadding(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-cellspacing
    KRYS_NODISCARD DOMString CellSpacing() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-cellspacing
    void CellSpacing(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLTableElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();