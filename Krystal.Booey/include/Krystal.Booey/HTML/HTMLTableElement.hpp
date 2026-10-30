#pragma once

#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLTableCaptionElement;
  class HTMLTableRowElement;
  class HTMLTableSectionElement;

  /// @see https://html.spec.whatwg.org/#htmltableelement
  class HTMLTableElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableElement);

  public:
    HTMLTableElement(dom::Document &document) noexcept;
    ~HTMLTableElement() noexcept override;

#pragma region HTMLTableElement - https://html.spec.whatwg.org/#htmltableelement

    /// @see https://html.spec.whatwg.org/#dom-table-caption
    KRYS_NODISCARD RefPtr<HTMLTableCaptionElement> Caption() const noexcept;

    /// @brief Removes the table's caption, and inserts @p caption as its first child if it isn't null.
    /// @see https://html.spec.whatwg.org/#dom-table-caption
    dom::ExceptionOr<void> Caption(RefPtr<HTMLTableCaptionElement> &&caption) noexcept;

    /// @brief The table's caption, made its first child if it had none.
    /// @see https://html.spec.whatwg.org/#dom-table-createcaption
    Ref<HTMLTableCaptionElement> CreateCaption() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-deletecaption
    void DeleteCaption() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-thead
    KRYS_NODISCARD RefPtr<HTMLTableSectionElement> THead() const noexcept;

    /// @brief Removes the table's `thead`, and inserts @p head after the caption and column groups if it
    /// isn't null: a `HierarchyRequestError` if it isn't a `thead`.
    /// @see https://html.spec.whatwg.org/#dom-table-thead
    dom::ExceptionOr<void> THead(RefPtr<HTMLTableSectionElement> &&head) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-createthead
    Ref<HTMLTableSectionElement> CreateTHead() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-deletethead
    void DeleteTHead() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-tfoot
    KRYS_NODISCARD RefPtr<HTMLTableSectionElement> TFoot() const noexcept;

    /// @brief Removes the table's `tfoot`, and appends @p foot if it isn't null: a `HierarchyRequestError` if
    /// it isn't a `tfoot`.
    /// @see https://html.spec.whatwg.org/#dom-table-tfoot
    dom::ExceptionOr<void> TFoot(RefPtr<HTMLTableSectionElement> &&foot) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-createtfoot
    Ref<HTMLTableSectionElement> CreateTFoot() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-deletetfoot
    void DeleteTFoot() noexcept;

    /// @brief The table's `tbody` children.
    /// @see https://html.spec.whatwg.org/#dom-table-tbodies
    KRYS_NODISCARD Ref<dom::HTMLCollection> TBodies() noexcept;

    /// @brief A new `tbody`, inserted after the table's last `tbody`, or at its end.
    /// @see https://html.spec.whatwg.org/#dom-table-createtbody
    Ref<HTMLTableSectionElement> CreateTBody() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-rows
    KRYS_NODISCARD Ref<dom::HTMLCollection> Rows() noexcept;

    /// @brief A new row at @p index in the table's rows (-1 for the end), in a new `tbody` if the table has
    /// no rows or bodies: an `IndexSizeError` if @p index is less than -1 or more than the number of rows.
    /// @see https://html.spec.whatwg.org/#dom-table-insertrow
    dom::ExceptionOr<Ref<HTMLTableRowElement>> InsertRow(int32 index = -1) noexcept;

    /// @brief Removes the row at @p index in the table's rows (-1 for the last, if any): an `IndexSizeError`
    /// if there's no such row.
    /// @see https://html.spec.whatwg.org/#dom-table-deleterow
    dom::ExceptionOr<void> DeleteRow(int32 index) noexcept;

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

  public:
    KRYS_NODISCARD const css::MutableStyleProperties *
      AdditionalPresentationalHintStyle() const noexcept override;

    /// @brief The presentational hints the table's cells share (the `rules`, `border` and `cellpadding`
    /// attributes).
    KRYS_NODISCARD const css::MutableStyleProperties *AdditionalCellStyle() const noexcept;

    /// @brief The presentational hints the table's row groups (@p rows) or column groups share
    /// (`rules=groups`).
    KRYS_NODISCARD const css::MutableStyleProperties *AdditionalGroupStyle(bool rows) const noexcept;

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

  private:
    KRYS_NODISCARD HTMLTableSectionElement *LastBody() const noexcept;

    enum class TableRules : uint8
    {
      Unset,
      None,
      Groups,
      Rows,
      Cols,
      All
    };

    enum class CellBorders : uint8
    {
      No,
      Inset,
      Solid,
      SolidColsOnly,
      SolidRowsOnly
    };

    KRYS_NODISCARD CellBorders GetCellBorders() const noexcept;
    KRYS_NODISCARD Ref<css::MutableStyleProperties> CreateSharedCellStyle() const noexcept;

    uint32 _borderAttr {0};  // Sets a precise border width and creates an outset border for the table and for
                             // its cells.
    bool _frameAttr {false}; // Implies a thin border width if no border is set and then a certain set of
                             // solid/ hidden borders based off the value.
    TableRules _rulesAttr {TableRules::Unset}; // Implies a thin border width, a collapsing border model, and
                                               // all borders on the table becoming set to hidden (if
                                               // frame/border are present, to none otherwise).
    uint16 _padding {1};
    mutable RefPtr<css::MutableStyleProperties> _sharedCellStyle;
    WeakPtr<dom::HTMLCollection> _rows;
    WeakPtr<dom::HTMLCollection> _tBodies;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTableElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();