#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys
{
  enum class ColourSpace : uint8;
}

namespace krys::boo
{
  class Color;

  /// @brief Returns the color serialised according to CSS.
  /// @see https://drafts.csswg.org/css-color-4/#serializing-color-values
  KRYS_NODISCARD utf8_string SerialisationForCSS(const Color &color) noexcept;

  /// @brief Returns the color serialised according to HTML5.
  /// @see https://html.spec.whatwg.org/multipage/scripting.html#fill-and-stroke-styles (10 September 2015).
  KRYS_NODISCARD utf8_string SerialisationForHTML(const Color &color) noexcept;

  /// @brief Returns the color serialised for DumpRenderTree: #RRGGBB, #RRGGBBAA or the CSS serialisation.
  KRYS_NODISCARD utf8_string SerialisationForRenderTreeAsText(const Color &color) noexcept;

  /// @brief Returns the CSS name of the color space, as used by `color()`.
  KRYS_NODISCARD utf8_stringview Serialisation(ColourSpace colorSpace) noexcept;
}
