#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmloptionelement
  class HTMLOptionElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLOptionElement);

  public:
    HTMLOptionElement(Document &document) noexcept;

#pragma region HTMLOptionElement - https://html.spec.whatwg.org/#htmloptionelement

    /// @see https://html.spec.whatwg.org/#dom-option-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-disabled
    void Disabled(bool value) noexcept;

    // TODO(HTMLOPTION, HTML): Implement the form API.
    // readonly attribute HTMLFormElement? form;

    /// @see https://html.spec.whatwg.org/#dom-option-label
    KRYS_NODISCARD DOMString Label() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-label
    void Label(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-defaultselected
    KRYS_NODISCARD bool DefaultSelected() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-defaultselected
    void DefaultSelected(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-selected
    KRYS_NODISCARD bool Selected() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-selected
    void Selected(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-value
    KRYS_NODISCARD DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-value
    void Value(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-text
    KRYS_NODISCARD DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-text
    void Text(DOMString &&value) noexcept;

    // TODO(HTMLOPTION, HTML): Implement the index attribute.
    // readonly attribute long index;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLOptionElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLOptionElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();