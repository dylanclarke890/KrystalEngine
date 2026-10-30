#pragma once

#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/HTML/HTMLTablePartElement.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::html
{
  class HTMLTableRowElement;

  /// @see https://html.spec.whatwg.org/#htmltablesectionelement
  class HTMLTableSectionElement : public HTMLTablePartElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableSectionElement);

  private:
    WeakPtr<dom::HTMLCollection> _rows;

  public:
    HTMLTableSectionElement(dom::Document &document) noexcept;

#pragma region HTMLTableSectionElement - https://html.spec.whatwg.org/#htmltablesectionelement

    /// @brief The section's `tr` children.
    /// @see https://html.spec.whatwg.org/#dom-tbody-rows
    KRYS_NODISCARD Ref<dom::HTMLCollection> Rows() noexcept;

    /// @brief A new row at @p index in the section's rows (-1 for the end): an `IndexSizeError` if @p index
    /// is less than -1 or more than the number of rows.
    /// @see https://html.spec.whatwg.org/#dom-tbody-insertrow
    dom::ExceptionOr<Ref<HTMLTableRowElement>> InsertRow(int32 index = -1) noexcept;

    /// @brief Removes the row at @p index in the section's rows (-1 for the last, if any): an
    /// `IndexSizeError` if there's no such row.
    /// @see https://html.spec.whatwg.org/#dom-tbody-deleterow
    dom::ExceptionOr<void> DeleteRow(int32 index) noexcept;

#pragma endregion

#pragma region HTMLTableSectionElement Obsolete members - https://html.spec.whatwg.org/#HTMLTableSectionElement-partial

    /// @see https://html.spec.whatwg.org/#dom-tbody-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-char
    KRYS_NODISCARD dom::DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-char
    void Char(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-charoff
    KRYS_NODISCARD dom::DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-charoff
    void CharOff(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-valign
    KRYS_NODISCARD dom::DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-valign
    void VAlign(dom::DOMString &&value) noexcept;

#pragma endregion

  public:
    KRYS_NODISCARD const css::MutableStyleProperties *
      AdditionalPresentationalHintStyle() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTableSectionElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableSectionElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();