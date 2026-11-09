/*
 * Copyright (C) 2014 Frederic Wang (fred.wang@free.fr). All rights reserved.
 * Copyright (C) 2016 Igalia S.L. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Fonts/Glyph.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Text/HarfBuzz.hpp"

namespace krys::boo
{
  class Font;
  class FontPlatformData;

  /// @brief A font's OpenType MATH table: the constants, italic corrections, size variants and glyph
  /// assemblies math layout uses, read through HarfBuzz.
  /// @see https://learn.microsoft.com/en-us/typography/opentype/spec/math
  class OpenTypeMathData : public RefCounted<OpenTypeMathData>
  {
  private:
    text::HbUniquePtr<hb_font_t> _mathFont;

  public:
    KRYS_NODISCARD static Ref<OpenTypeMathData> Create(const FontPlatformData &font) noexcept
    {
      return AdoptRef(*new OpenTypeMathData(font));
    }

    ~OpenTypeMathData() noexcept;

    KRYS_NODISCARD bool HasMathData() const noexcept
    {
      return _mathFont != nullptr;
    }

    // These constants are defined in the MATH table.
    // The implementation of OpenTypeMathData::getMathConstant assumes that they correspond to the indices of
    // the MathContant table.
    enum class MathConstant : uint8
    {
      ScriptPercentScaleDown,
      ScriptScriptPercentScaleDown,
      DelimitedSubFormulaMinHeight,
      DisplayOperatorMinHeight,
      MathLeading,
      AxisHeight,
      AccentBaseHeight,
      FlattenedAccentBaseHeight,
      SubscriptShiftDown,
      SubscriptTopMax,
      SubscriptBaselineDropMin,
      SuperscriptShiftUp,
      SuperscriptShiftUpCramped,
      SuperscriptBottomMin,
      SuperscriptBaselineDropMax,
      SubSuperscriptGapMin,
      SuperscriptBottomMaxWithSubscript,
      SpaceAfterScript,
      UpperLimitGapMin,
      UpperLimitBaselineRiseMin,
      LowerLimitGapMin,
      LowerLimitBaselineDropMin,
      StackTopShiftUp,
      StackTopDisplayStyleShiftUp,
      StackBottomShiftDown,
      StackBottomDisplayStyleShiftDown,
      StackGapMin,
      StackDisplayStyleGapMin,
      StretchStackTopShiftUp,
      StretchStackBottomShiftDown,
      StretchStackGapAboveMin,
      StretchStackGapBelowMin,
      FractionNumeratorShiftUp,
      FractionNumeratorDisplayStyleShiftUp,
      FractionDenominatorShiftDown,
      FractionDenominatorDisplayStyleShiftDown,
      FractionNumeratorGapMin,
      FractionNumDisplayStyleGapMin,
      FractionRuleThickness,
      FractionDenominatorGapMin,
      FractionDenomDisplayStyleGapMin,
      SkewedFractionHorizontalGap,
      SkewedFractionVerticalGap,
      OverbarVerticalGap,
      OverbarRuleThickness,
      OverbarExtraAscender,
      UnderbarVerticalGap,
      UnderbarRuleThickness,
      UnderbarExtraDescender,
      RadicalVerticalGap,
      RadicalDisplayStyleVerticalGap,
      RadicalRuleThickness,
      RadicalExtraAscender,
      RadicalKernBeforeDegree,
      RadicalKernAfterDegree,
      RadicalDegreeBottomRaisePercent,
    };

    struct AssemblyPart
    {
      boo::Glyph Glyph {0};
      bool IsExtender {false};

      KRYS_NODISCARD bool operator==(const AssemblyPart &) const noexcept = default;
    };

    /// @brief The constant @p constant at @p font's size; the percentages are fractions.
    KRYS_NODISCARD float GetMathConstant(const Font &font, MathConstant constant) const noexcept;
    /// @brief The italic correction of @p glyph at @p font's size.
    KRYS_NODISCARD float GetItalicCorrection(const Font &font, Glyph glyph) const noexcept;
    /// @brief The larger versions of @p glyph in the direction it stretches, and the parts to build it from
    /// when they aren't large enough.
    void GetMathVariants(Glyph glyph, bool isVertical, List<Glyph> &sizeVariants,
                         List<AssemblyPart> &assemblyParts) const noexcept;

  private:
    explicit OpenTypeMathData(const FontPlatformData &font) noexcept;
  };
}
