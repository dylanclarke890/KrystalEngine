#pragma once

#include "Krystal.Booey/Html/Elements/HTMLTablePartElement.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::boo
{
  class HTMLCollection;
  class HTMLTableCellElement;

  /// @see https://html.spec.whatwg.org/multipage/tables.html#htmltablerowelement
  class HTMLTableRowElement : public HTMLTablePartElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableRowElement);

  private:
    WeakPtr<HTMLCollection> _cells;

  public:
    HTMLTableRowElement(Document &document) noexcept;

#pragma region HTMLTableRowElement - https://html.spec.whatwg.org/multipage/tables.html#htmltablerowelement

    /// @brief The row's index in its table's rows, or -1 if it isn't in a table.
    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tr-rowindex
    KRYS_NODISCARD int32 RowIndex() const noexcept;

    /// @brief The row's index in its section's rows (or its table's, if it's a child of the table), or -1.
    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tr-sectionrowindex
    KRYS_NODISCARD int32 SectionRowIndex() const noexcept;

    /// @brief The row's `td` and `th` children.
    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tr-cells
    KRYS_NODISCARD Ref<HTMLCollection> Cells() noexcept;

    /// @brief A new `td` at @p index in the row's cells (-1 for the end): an `IndexSizeError` if @p index is
    /// less than -1 or more than the number of cells.
    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tr-insertcell
    ExceptionOr<Ref<HTMLTableCellElement>> InsertCell(int32 index = -1) noexcept;

    /// @brief Removes the cell at @p index in the row's cells (-1 for the last, if any): an `IndexSizeError`
    /// if there's no such cell.
    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tr-deletecell
    ExceptionOr<void> DeleteCell(int32 index) noexcept;

#pragma endregion

#pragma region HTMLTableRowElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLTableRowElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tr-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tr-align
    void Align(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-char
    KRYS_NODISCARD DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-char
    void Char(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-charoff
    KRYS_NODISCARD DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tr-charoff
    void CharOff(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tr-valign
    KRYS_NODISCARD DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tr-valign
    void VAlign(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tr-bgcolor
    KRYS_NODISCARD DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tr-bgcolor
    void BgColor(DOMString value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLTableRowElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return Is<krys::boo::HTMLElement>(target)
           && Downcast<krys::boo::HTMLElement>(target).IsHTMLTableRowElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableRowElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
