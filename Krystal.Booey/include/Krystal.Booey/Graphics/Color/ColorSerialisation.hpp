#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class Color;

  enum class ColorSpace : uint8;

  /// @brief returns the color serialised according to CSS.
  /// @see https://drafts.csswg.org/css-color-4/#serializing-color-values
  KRYS_NODISCARD utf8_string SerialisationForCSS(const Color &color) noexcept;

  /// @brief returns the color serialised according to HTML5/
  /// @see https://html.spec.whatwg.org/multipage/scripting.html#fill-and-stroke-styles (10 September 2015).
  KRYS_NODISCARD utf8_string SerialisationForHTML(const Color &color) noexcept;

  /// @brief returns the color serialised for DumpRenderTree, #RRGGBB, #RRGGBBAA or the CSS serialisation.
  KRYS_NODISCARD utf8_string SerialisationForRenderTreeAsText(const Color &color) noexcept;

  KRYS_NODISCARD utf8_string Serialisation(ColorSpace colorSpace) noexcept;
}
