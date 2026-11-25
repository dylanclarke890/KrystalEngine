#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"

namespace krys::boo
{
  class HTMLCollection;
  class HTMLTableCaptionElement;
  class HTMLTableRowElement;
  class HTMLTableSectionElement;

  /// @see https://html.spec.whatwg.org/#htmltableelement
  class HTMLTableElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableElement);

  public:
    HTMLTableElement(Document &document) noexcept;

    ~HTMLTableElement() noexcept override;

#pragma region HTMLTableElement - https://html.spec.whatwg.org/#htmltableelement

    /// @see https://html.spec.whatwg.org/#dom-table-caption
    KRYS_NODISCARD RefPtr<HTMLTableCaptionElement> Caption() const noexcept;

    /// @brief Removes the table's caption, and inserts @p caption as its first child if it isn't null.
    /// @see https://html.spec.whatwg.org/#dom-table-caption
    ExceptionOr<void> Caption(const RefPtr<HTMLTableCaptionElement> &caption) noexcept;

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
    ExceptionOr<void> THead(const RefPtr<HTMLTableSectionElement> &head) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-createthead
    Ref<HTMLTableSectionElement> CreateTHead() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-deletethead
    void DeleteTHead() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-tfoot
    KRYS_NODISCARD RefPtr<HTMLTableSectionElement> TFoot() const noexcept;

    /// @brief Removes the table's `tfoot`, and appends @p foot if it isn't null: a `HierarchyRequestError` if
    /// it isn't a `tfoot`.
    /// @see https://html.spec.whatwg.org/#dom-table-tfoot
    ExceptionOr<void> TFoot(const RefPtr<HTMLTableSectionElement> &foot) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-createtfoot
    Ref<HTMLTableSectionElement> CreateTFoot() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-deletetfoot
    void DeleteTFoot() noexcept;

    /// @brief The table's `tbody` children.
    /// @see https://html.spec.whatwg.org/#dom-table-tbodies
    KRYS_NODISCARD Ref<HTMLCollection> TBodies() noexcept;

    /// @brief A new `tbody`, inserted after the table's last `tbody`, or at its end.
    /// @see https://html.spec.whatwg.org/#dom-table-createtbody
    Ref<HTMLTableSectionElement> CreateTBody() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-rows
    KRYS_NODISCARD Ref<HTMLCollection> Rows() noexcept;

    /// @brief A new row at @p index in the table's rows (-1 for the end), in a new `tbody` if the table has
    /// no rows or bodies: an `IndexSizeError` if @p index is less than -1 or more than the number of rows.
    /// @see https://html.spec.whatwg.org/#dom-table-insertrow
    ExceptionOr<Ref<HTMLTableRowElement>> InsertRow(int32 index = -1) noexcept;

    /// @brief Removes the row at @p index in the table's rows (-1 for the last, if any): an `IndexSizeError`
    /// if there's no such row.
    /// @see https://html.spec.whatwg.org/#dom-table-deleterow
    ExceptionOr<void> DeleteRow(int32 index) noexcept;

#pragma endregion

#pragma region HTMLTableElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableElement-partial

    /// @see https://html.spec.whatwg.org/#dom-table-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-align
    void Align(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-border
    KRYS_NODISCARD DOMString Border() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-border
    void Border(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-frame
    KRYS_NODISCARD DOMString Frame() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-frame
    void Frame(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-rules
    KRYS_NODISCARD DOMString Rules() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-rules
    void Rules(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-summary
    KRYS_NODISCARD DOMString Summary() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-summary
    void Summary(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-width
    void Width(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-bgcolor
    KRYS_NODISCARD DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-bgcolor
    void BgColor(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-cellpadding
    KRYS_NODISCARD DOMString CellPadding() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-cellpadding
    void CellPadding(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-cellspacing
    KRYS_NODISCARD DOMString CellSpacing() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-table-cellspacing
    void CellSpacing(DOMString value) noexcept;

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
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

  private:
    /// @brief The table's last tbody child, if it has one.
    KRYS_NODISCARD HTMLTableSectionElement *LastBody() const noexcept;

    enum class TableRules : uint8
    {
      Unset,
      None,
      Groups,
      Rows,
      Cols,
      All,
    };

    enum class CellBorders : uint8
    {
      No,
      Inset,
      Solid,
      SolidColsOnly,
      SolidRowsOnly,
    };

    /// @brief The borders the cells get from the table's border, frame and rules attributes.
    KRYS_NODISCARD CellBorders GetCellBorders() const noexcept;

    /// @brief The presentational hints the table gives all its cells: their borders and padding.
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
    WeakPtr<HTMLCollection> _rows;
    WeakPtr<HTMLCollection> _tBodies;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLTableElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return Is<krys::boo::HTMLElement>(target)
           && Downcast<krys::boo::HTMLElement>(target).IsHTMLTableElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
