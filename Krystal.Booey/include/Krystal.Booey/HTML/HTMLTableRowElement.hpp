#pragma once

#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/HTML/HTMLTablePartElement.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLTableCellElement;

  /// @see https://html.spec.whatwg.org/#htmltablerowelement
  class HTMLTableRowElement : public HTMLTablePartElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableRowElement);

  private:
    WeakPtr<dom::HTMLCollection> _cells;

  public:
    HTMLTableRowElement(dom::Document &document) noexcept;

#pragma region HTMLTableRowElement - https://html.spec.whatwg.org/#htmltablerowelement

    /// @brief The row's index in its table's rows, or -1 if it isn't in a table.
    /// @see https://html.spec.whatwg.org/#dom-tr-rowindex
    KRYS_NODISCARD int32 RowIndex() const noexcept;

    /// @brief The row's index in its section's rows (or its table's, if it's a child of the table), or -1.
    /// @see https://html.spec.whatwg.org/#dom-tr-sectionrowindex
    KRYS_NODISCARD int32 SectionRowIndex() const noexcept;

    /// @brief The row's `td` and `th` children.
    /// @see https://html.spec.whatwg.org/#dom-tr-cells
    KRYS_NODISCARD Ref<dom::HTMLCollection> Cells() noexcept;

    /// @brief A new `td` at @p index in the row's cells (-1 for the end): an `IndexSizeError` if @p index is
    /// less than -1 or more than the number of cells.
    /// @see https://html.spec.whatwg.org/#dom-tr-insertcell
    dom::ExceptionOr<Ref<HTMLTableCellElement>> InsertCell(int32 index = -1) noexcept;

    /// @brief Removes the cell at @p index in the row's cells (-1 for the last, if any): an `IndexSizeError`
    /// if there's no such cell.
    /// @see https://html.spec.whatwg.org/#dom-tr-deletecell
    dom::ExceptionOr<void> DeleteCell(int32 index) noexcept;

#pragma endregion

#pragma region HTMLTableRowElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableRowElement-partial

    /// @see https://html.spec.whatwg.org/#dom-tr-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-char
    KRYS_NODISCARD dom::DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-char
    void Char(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-charoff
    KRYS_NODISCARD dom::DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-charoff
    void CharOff(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-valign
    KRYS_NODISCARD dom::DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-valign
    void VAlign(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-bgcolor
    KRYS_NODISCARD dom::DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-bgcolor
    void BgColor(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTableRowElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableRowElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();