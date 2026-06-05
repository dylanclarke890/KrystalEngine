#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlmeterelement
  class HTMLMeterElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMeterElement);

  public:
    HTMLMeterElement(Document &document) noexcept;

#pragma region HTMLMeterElement - https://html.spec.whatwg.org/#htmlmeterelement

    /// @see https://html.spec.whatwg.org/#dom-meter-value
    KRYS_NODISCARD double Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-value
    void Value(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-min
    KRYS_NODISCARD double Min() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-min
    void Min(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-max
    KRYS_NODISCARD double Max() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-max
    void Max(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-low
    KRYS_NODISCARD double Low() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-low
    void Low(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-high
    KRYS_NODISCARD double High() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-high
    void High(double value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-optimum
    KRYS_NODISCARD double Optimum() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-meter-optimum
    void Optimum(double value) noexcept;

    // TODO(HTMLMETER, HTML): Implement Labels().
    // readonly attribute NodeList labels;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLMeterElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLMeterElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
