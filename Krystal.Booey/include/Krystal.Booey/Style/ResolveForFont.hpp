#pragma once

#include "Krystal.Booey/Graphics/FontSelectionAlgorithm.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class FontStyleWithAngleValue;
  class Value;
}

namespace krys::boo::style
{
  // FIXME: `resolveForUnresolvedFont`, for the canvas 2D context's `font` and the `FontFace` interface, once
  // there are those.

  /// @brief The weight of a `font-weight` (or `@font-face` `font-weight`) number or keyword.
  KRYS_NODISCARD FontSelectionValue FontWeightFromCSSValueDeprecated(const css::Value &value) noexcept;
  /// @brief The width of a `font-width` (or `font-stretch`) percentage or keyword.
  KRYS_NODISCARD FontSelectionValue FontStretchFromCSSValueDeprecated(const css::Value &value) noexcept;
  /// @brief The slope of an oblique angle.
  KRYS_NODISCARD FontSelectionValue FontStyleAngleFromCSSValueDeprecated(const css::Value &value) noexcept;
  KRYS_NODISCARD Maybe<FontSelectionValue> FontStyleAngleFromCSSFontStyleWithAngleValueDeprecated(
    const css::FontStyleWithAngleValue &value) noexcept;
  /// @brief The slope of a `font-style`: null for `normal`.
  KRYS_NODISCARD Maybe<FontSelectionValue> FontStyleFromCSSValueDeprecated(const css::Value &value) noexcept;
}
