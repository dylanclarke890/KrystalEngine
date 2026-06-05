#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmloptgroupelement
  class HTMLOptGroupElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLOptGroupElement);

  public:
    HTMLOptGroupElement(Document &document) noexcept;

#pragma region HTMLOptGroupElement - https://html.spec.whatwg.org/#htmloptgroupelement

    /// @see https://html.spec.whatwg.org/#dom-optgroup-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-optgroup-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-optgroup-label
    KRYS_NODISCARD DOMString Label() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-optgroup-label
    void Label(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLOptGroupElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLOptGroupElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();