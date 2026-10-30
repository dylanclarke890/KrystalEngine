#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  struct BlendingContext;
  class Color;

  // NOTE: These functions do a lossy conversion to 8-bit sRGBA before blending.

  /// @brief An implementation of Porter-Duff's "source-over" equation.
  KRYS_NODISCARD Color BlendSourceOver(const Color &backdrop, const Color &source) noexcept;

  /// @brief Bespoke "whitening" algorithm used by RenderTheme::transformSelectionBackgroundColor.
  /// @note This is a no-op if the color to blend with isn't opaque, which is likely not what you were
  /// expecting.
  KRYS_NODISCARD Color BlendWithWhite(const Color &color) noexcept;

  KRYS_NODISCARD Color Blend(const Color &from, const Color &to, const BlendingContext &context) noexcept;

  KRYS_NODISCARD Color BlendWithoutPremultiply(const Color &from, const Color &to,
                                               const BlendingContext &context) noexcept;
}
