#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlhrelement
  class HTMLHRElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHRElement);

  public:
    HTMLHRElement(Document &document) noexcept;

#pragma region HTMLHRElement Obsolete members - https://html.spec.whatwg.org/#HTMLHRElement-partial

    /// @see https://html.spec.whatwg.org/#dom-hr-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-align
    void Align(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-color
    KRYS_NODISCARD DOMString Color() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-color
    void Color(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-noshade
    KRYS_NODISCARD bool NoShade() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-noshade
    void NoShade(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-size
    KRYS_NODISCARD DOMString Size() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-size
    void Size(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-width
    void Width(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLHRElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLHRElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();