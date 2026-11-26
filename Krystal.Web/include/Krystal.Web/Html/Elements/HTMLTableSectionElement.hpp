#pragma once

#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Web/Html/Elements/HTMLTablePartElement.hpp"

namespace krys::web
{
  class HTMLCollection;
  class HTMLTableRowElement;

  /// @see https://html.spec.whatwg.org/multipage/tables.html#htmltablesectionelement
  class HTMLTableSectionElement : public HTMLTablePartElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTableSectionElement);

  private:
    WeakPtr<HTMLCollection> _rows;

  public:
    HTMLTableSectionElement(Document &document) noexcept;

#pragma region HTMLTableSectionElement - https://html.spec.whatwg.org/multipage/tables.html#htmltablesectionelement

    /// @brief The section's `tr` children.
    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tbody-rows
    KRYS_NODISCARD Ref<HTMLCollection> Rows() noexcept;

    /// @brief A new row at @p index in the section's rows (-1 for the end): an `IndexSizeError` if @p index
    /// is less than -1 or more than the number of rows.
    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tbody-insertrow
    ExceptionOr<Ref<HTMLTableRowElement>> InsertRow(int32 index = -1) noexcept;

    /// @brief Removes the row at @p index in the section's rows (-1 for the last, if any): an
    /// `IndexSizeError` if there's no such row.
    /// @see https://html.spec.whatwg.org/multipage/tables.html#dom-tbody-deleterow
    ExceptionOr<void> DeleteRow(int32 index) noexcept;

#pragma endregion

#pragma region HTMLTableSectionElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLTableSectionElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tbody-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tbody-align
    void Align(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-char
    KRYS_NODISCARD DOMString Char() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-char
    void Char(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-charoff
    KRYS_NODISCARD DOMString CharOff() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tbody-charoff
    void CharOff(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tbody-valign
    KRYS_NODISCARD DOMString VAlign() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-tbody-valign
    void VAlign(DOMString value) noexcept;

#pragma endregion

  public:
    KRYS_NODISCARD const css::MutableStyleProperties *
      AdditionalPresentationalHintStyle() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLTableSectionElement)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return Is<krys::web::HTMLElement>(target)
           && Downcast<krys::web::HTMLElement>(target).IsHTMLTableSectionElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLTableSectionElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
