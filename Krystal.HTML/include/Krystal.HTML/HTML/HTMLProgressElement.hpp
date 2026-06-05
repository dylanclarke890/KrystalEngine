#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlprogresselement
  class HTMLProgressElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLProgressElement);

  public:
    HTMLProgressElement(Document &document) noexcept;

#pragma region HTMLProgressElement - https://html.spec.whatwg.org/#htmlprogresselement

    /// @see https://html.spec.whatwg.org/#dom-progress-value
    KRYS_NODISCARD double Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-progress-value
    void Value(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-progress-max
    KRYS_NODISCARD double Max() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-progress-max
    void Max(double value) noexcept;

    // readonly attribute double position;
    // readonly attribute NodeList labels;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLProgressElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLProgressElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
