#pragma once

#include "Krystal.Booey/DOM/FocusOptions.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::html::mixins
{
  /// @brief The members HTML, SVG and MathML elements share: focus and the attributes behind it.
  /// @see https://html.spec.whatwg.org/#htmlorsvgormathmlelement
  // FIXME: `dataset` and `nonce`.
  class HTMLOrSVGOrMathMLElement
  {
  public:
    /// @see https://html.spec.whatwg.org/#dom-tabindex
    KRYS_NODISCARD static int32 TabIndex(const dom::Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tabindex
    static void TabIndex(dom::Element &element, int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-fe-autofocus
    KRYS_NODISCARD static bool Autofocus(const dom::Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-fe-autofocus
    static void Autofocus(dom::Element &element, bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-focus
    static void Focus(dom::Element &element, const dom::FocusOptions &options = {}) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-blur
    static void Blur(dom::Element &element) noexcept;
  };
}
