#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmltableelement
  class HTMLTableElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableElement);

  public:
    HTMLTableElement(dom::Document &document) noexcept;

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
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-border
    KRYS_NODISCARD dom::DOMString Border() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-border
    void Border(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-frame
    KRYS_NODISCARD dom::DOMString Frame() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-frame
    void Frame(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-rules
    KRYS_NODISCARD dom::DOMString Rules() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-rules
    void Rules(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-summary
    KRYS_NODISCARD dom::DOMString Summary() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-summary
    void Summary(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-width
    KRYS_NODISCARD dom::DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-width
    void Width(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-bgcolor
    KRYS_NODISCARD dom::DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-bgcolor
    void BgColor(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-cellpadding
    KRYS_NODISCARD dom::DOMString CellPadding() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-cellpadding
    void CellPadding(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-cellspacing
    KRYS_NODISCARD dom::DOMString CellSpacing() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-cellspacing
    void CellSpacing(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTableElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();