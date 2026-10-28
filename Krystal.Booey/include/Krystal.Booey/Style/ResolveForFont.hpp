#pragma once

#include "Krystal.Booey/Graphics/FontSelectionAlgorithm.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class FontCascade;
  class FontCascadeDescription;
}

namespace krys::boo::css
{
  class FontStyleWithAngleValue;
  class Value;

  namespace PropertyParserHelpers
  {
    struct UnresolvedFont;
  }
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::style
{
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

  /// @brief @p unresolvedFont's longhands applied over @p fontDescription (the font they're relative to), and
  /// the fonts of @p document's font selector; empty if it names no families.
  /// @note What the canvas 2D context's `font` is resolved with.
  KRYS_NODISCARD Maybe<FontCascade>
    ResolveForUnresolvedFont(const css::PropertyParserHelpers::UnresolvedFont &unresolvedFont,
                             FontCascadeDescription &&fontDescription, dom::Document &document) noexcept;
}
