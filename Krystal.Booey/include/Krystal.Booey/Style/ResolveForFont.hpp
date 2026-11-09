/*
 * Copyright (C) 2020 Metrological Group B.V.
 * Copyright (C) 2020 Igalia S.L.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Fonts/FontSelectionAlgorithm.hpp"
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
