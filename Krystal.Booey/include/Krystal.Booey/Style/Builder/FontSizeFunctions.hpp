/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  class FontMetrics;
  class RenderStyle;
  struct FontSizeAdjust;
  struct SettingsValues;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::style
{
  enum class MinimumFontSizeRule : uint8
  {
    None,
    Absolute,
    AbsoluteAndRelative
  };

  struct ComputedFontSize
  {
    float Size {0.0f};
    float UsedZoomFactor {1.0f};
  };

  KRYS_NODISCARD float ComputedFontSizeFromSpecifiedSize(float specifiedSize, bool isAbsoluteSize,
                                                         float zoomFactor, MinimumFontSizeRule,
                                                         const SettingsValues &) noexcept;
  KRYS_NODISCARD ComputedFontSize ComputedFontSizeFromSpecifiedSize(float specifiedSize, bool isAbsoluteSize,
                                                                    bool useSVGZoomRules, const RenderStyle *,
                                                                    const dom::Document &) noexcept;
  KRYS_NODISCARD float ComputedFontSizeFromSpecifiedSizeForSVGInlineText(float specifiedSize,
                                                                         bool isAbsoluteSize,
                                                                         float zoomFactor,
                                                                         const dom::Document &) noexcept;
  KRYS_NODISCARD float AdjustedFontSize(float size, const boo::FontSizeAdjust &,
                                        const FontMetrics &) noexcept;

  // Given a CSS keyword id in the range (CSSValueXxSmall to CSSValueXxxLarge), this function will return
  // the correct font size scaled relative to the user's default (medium).
  KRYS_NODISCARD float FontSizeForKeyword(uint32 keywordID, bool shouldUseFixedDefaultSize,
                                          const SettingsValues &, bool inQuirksMode = false) noexcept;
  KRYS_NODISCARD float FontSizeForKeyword(uint32 keywordID, bool shouldUseFixedDefaultSize,
                                          const dom::Document &) noexcept;

  // Given a font size in pixel, this function will return legacy font size between 1 and 7.
  KRYS_NODISCARD int32 LegacyFontSizeForPixelSize(int32 pixelFontSize, bool shouldUseFixedDefaultSize,
                                                  const dom::Document &) noexcept;
}
