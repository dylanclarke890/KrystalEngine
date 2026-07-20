#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmldialogelement
  class HTMLDialogElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDialogElement);

  public:
    HTMLDialogElement(Document &document) noexcept;

#pragma region HTMLDialogElement - https://html.spec.whatwg.org/#htmldialogelement

    /// @see https://html.spec.whatwg.org/#dom-dialog-open
    KRYS_NODISCARD bool Open() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-open
    void Open(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-returnvalue
    KRYS_NODISCARD DOMString ReturnValue() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-returnvalue
    void ReturnValue(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-closedby
    KRYS_NODISCARD DOMString ClosedBy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-closedby
    void ClosedBy(DOMString &&value) noexcept;

    // TODO(HTMLDIALOG, HTML): Implement the form API.
    // [CEReactions] undefined show();
    // [CEReactions] undefined showModal();
    // [CEReactions] undefined close(optional DOMString returnValue);
    // [CEReactions] undefined requestClose(optional DOMString returnValue);

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLDialogElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLDialogElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();