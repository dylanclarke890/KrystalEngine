#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmldialogelement
  class HTMLDialogElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDialogElement);

  public:
    HTMLDialogElement(dom::Document &document) noexcept;

#pragma region HTMLDialogElement - https://html.spec.whatwg.org/#htmldialogelement

    /// @see https://html.spec.whatwg.org/#dom-dialog-open
    KRYS_NODISCARD bool Open() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-open
    void Open(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-returnvalue
    KRYS_NODISCARD dom::DOMString ReturnValue() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-returnvalue
    void ReturnValue(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-closedby
    KRYS_NODISCARD dom::DOMString ClosedBy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dialog-closedby
    void ClosedBy(dom::DOMString &&value) noexcept;

    // TODO(HTMLDIALOG, HTML): Implement the form API.
    // [CEReactions] undefined show();
    // [CEReactions] undefined showModal();
    // [CEReactions] undefined close(optional dom::DOMString returnValue);
    // [CEReactions] undefined requestClose(optional dom::DOMString returnValue);

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLDialogElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLDialogElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();