#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Web/Html/Algorithms/FocusOptions.hpp"

namespace krys::web
{
  class Element;
}

namespace krys::web::html::mixins
{
  /// @brief The members HTML, SVG and MathML elements share: focus and the attributes behind it.
  /// @see https://html.spec.whatwg.org/#htmlorsvgormathmlelement
  /// @note `dataset` is `Element::Dataset()`.
  class HTMLOrSVGOrMathMLElement
  {
  public:
    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-tabindex
    KRYS_NODISCARD static int32 TabIndex(const Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-tabindex
    static void TabIndex(Element &element, int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-fe-autofocus
    KRYS_NODISCARD static bool Autofocus(const Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-fe-autofocus
    static void Autofocus(Element &element, bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-focus
    static void Focus(Element &element, const html::FocusOptions &options = {}) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-blur
    static void Blur(Element &element) noexcept;
  };
}
