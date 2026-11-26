#pragma once

#include "Krystal.Booey/Html/Elements/HTMLTablePartElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/tables.html#htmltablecellelement
  class HTMLTableCellElement : public HTMLTablePartElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableCellElement);

  public:
    HTMLTableCellElement(Document &document) noexcept;

#pragma region HTMLTableCellElement - https://html.spec.whatwg.org/multipage/tables.html#htmltablecellelement

    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tdth-colspan
    KRYS_NODISCARD uint32 ColSpan() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tdth-colspan
    void ColSpan(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tdth-rowspan
    KRYS_NODISCARD uint32 RowSpan() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tdth-rowspan
    void RowSpan(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tdth-headers
    KRYS_NODISCARD DOMString Headers() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tdth-headers
    void Headers(DOMString value) noexcept;

    /// @brief The cell's index in its row's cells, or -1 if its parent isn't a row.
    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tdth-cellindex
    KRYS_NODISCARD int32 CellIndex() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-scope
    KRYS_NODISCARD DOMString Scope() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-scope
    void Scope(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-abbr
    KRYS_NODISCARD DOMString Abbr() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-abbr
    void Abbr(DOMString value) noexcept;

#pragma endregion

#pragma region HTMLTableCellElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLTableCellElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-align
    void Align(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-axis
    KRYS_NODISCARD DOMString Axis() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-axis
    void Axis(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-height
    KRYS_NODISCARD DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-height
    void Height(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-width
    void Width(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-char
    KRYS_NODISCARD DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-char
    void Char(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-charoff
    KRYS_NODISCARD DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tdth-charoff
    void CharOff(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-nowrap
    KRYS_NODISCARD bool NoWrap() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-nowrap
    void NoWrap(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-valign
    KRYS_NODISCARD DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-valign
    void VAlign(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-bgcolor
    KRYS_NODISCARD DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tdth-bgcolor
    void BgColor(DOMString value) noexcept;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLTableCellElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return Is<krys::boo::HTMLElement>(target)
           && Downcast<krys::boo::HTMLElement>(target).IsHTMLTableCellElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableCellElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
