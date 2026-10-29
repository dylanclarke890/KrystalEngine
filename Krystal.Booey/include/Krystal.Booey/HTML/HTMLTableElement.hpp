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
    ~HTMLTableElement() noexcept override;

#pragma region HTMLTableElement - https://html.spec.whatwg.org/#htmltableelement

    // FIXME: Implement Caption(), CreateCaption(), DeleteCaption(), THead(),
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
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTableElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();