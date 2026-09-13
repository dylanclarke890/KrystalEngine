#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlprogresselement
  class HTMLProgressElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLProgressElement);

  public:
    HTMLProgressElement(dom::Document &document) noexcept;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLProgressElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLProgressElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
