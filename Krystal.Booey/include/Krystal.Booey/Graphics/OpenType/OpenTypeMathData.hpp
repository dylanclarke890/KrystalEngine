#pragma once

#include "Krystal.Booey/Graphics/Glyph.hpp"
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
