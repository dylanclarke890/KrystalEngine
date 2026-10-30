#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/GlyphPage.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/OpenType/OpenTypeMathData.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  struct PaintInfo;
  class RenderStyle;

  /// @brief The glyphs of an operator character: its base glyph, a larger size variant, or a glyph assembly
  /// stretched to a size, measured and painted.
  class MathOperator
  {
  public:
    enum class Type : uint8
    {
      NormalOperator,
      DisplayOperator,
      VerticalOperator,
      HorizontalOperator
    };

  private:
    struct GlyphAssemblyData
    {
      char32 TopOrRightCodePoint {0};
      Glyph TopOrRightFallbackGlyph {0};
      char32 ExtensionCodePoint {0};
      Glyph ExtensionFallbackGlyph {0};
      char32 BottomOrLeftCodePoint {0};
      Glyph BottomOrLeftFallbackGlyph {0};
      char32 MiddleCodePoint {0};
      Glyph MiddleFallbackGlyph {0};

      KRYS_NODISCARD bool HasExtension() const noexcept
      {
        return ExtensionCodePoint || ExtensionFallbackGlyph;
      }
      KRYS_NODISCARD bool HasMiddle() const noexcept
      {
        return MiddleCodePoint || MiddleFallbackGlyph;
      }
    };

    enum class StretchType : uint8
    {
      Unstretched,
      SizeVariant,
      GlyphAssembly
    };

    enum class GlyphPaintTrimming : uint8
    {
      TrimTop,
      TrimBottom,
      TrimTopAndBottom,
      TrimLeft,
      TrimRight,
      TrimLeftAndRight
    };

    char32 _baseCharacter {0};
    Type _operatorType {Type::NormalOperator};
    StretchType _stretchType {StretchType::Unstretched};
    Glyph _variantGlyph {0};
    GlyphAssemblyData _assembly;
    LayoutUnit _maxPreferredWidth {0};
    LayoutUnit _width {0};
    LayoutUnit _ascent {0};
    LayoutUnit _descent {0};
    LayoutUnit _italicCorrection {0};
    float _radicalVerticalScale {1};

  public:
    MathOperator() noexcept = default;

    void SetOperator(const RenderStyle &style, char32 baseCharacter, Type operatorType) noexcept;
    void Reset(const RenderStyle &style) noexcept;

    KRYS_NODISCARD LayoutUnit Width() const noexcept
    {
      return _width;
    }
    KRYS_NODISCARD LayoutUnit MaxPreferredWidth() const noexcept
    {
      return _maxPreferredWidth;
    }
    KRYS_NODISCARD LayoutUnit Ascent() const noexcept
    {
      return _ascent;
    }
    KRYS_NODISCARD LayoutUnit Descent() const noexcept
    {
      return _descent;
    }
    KRYS_NODISCARD LayoutUnit ItalicCorrection() const noexcept
    {
      return _italicCorrection;
    }

    void StretchTo(const RenderStyle &style, LayoutUnit targetSize) noexcept;

    void Paint(const RenderStyle &style, PaintInfo &info, const LayoutPoint &paintOffset,
               float deviceScaleFactor) noexcept;

  private:
    KRYS_NODISCARD LayoutUnit StretchSize() const noexcept;
    KRYS_NODISCARD bool GetGlyph(const RenderStyle &style, char32 character, GlyphData &glyph) const noexcept;
    KRYS_NODISCARD bool GetBaseGlyph(const RenderStyle &style, GlyphData &baseGlyph) const noexcept
    {
      return GetGlyph(style, _baseCharacter, baseGlyph);
    }
    void SetSizeVariant(const GlyphData &sizeVariant) noexcept;
    void SetGlyphAssembly(const RenderStyle &style, const GlyphAssemblyData &assemblyData) noexcept;
    void GetMathVariantsWithFallback(const RenderStyle &style, bool isVertical, List<Glyph> &sizeVariants,
                                     List<OpenTypeMathData::AssemblyPart> &assemblyParts) const noexcept;
    void CalculateDisplayStyleLargeOperator(const RenderStyle &style) noexcept;
    void CalculateStretchyData(const RenderStyle &style, bool calculateMaxPreferredWidth,
                               LayoutUnit targetSize = 0_lu) noexcept;
    KRYS_NODISCARD bool
      CalculateGlyphAssemblyFallback(const List<OpenTypeMathData::AssemblyPart> &assemblyParts,
                                     GlyphAssemblyData &assemblyData) const noexcept;

    LayoutRect PaintGlyph(const RenderStyle &style, PaintInfo &info, const GlyphData &data,
                          const LayoutPoint &origin, GlyphPaintTrimming trim) noexcept;
    void FillWithVerticalExtensionGlyph(const RenderStyle &style, PaintInfo &info, const LayoutPoint &from,
                                        const LayoutPoint &to) noexcept;
    void FillWithHorizontalExtensionGlyph(const RenderStyle &style, PaintInfo &info, const LayoutPoint &from,
                                          const LayoutPoint &to) noexcept;
    void PaintVerticalGlyphAssembly(const RenderStyle &style, PaintInfo &info,
                                    const LayoutPoint &paintOffset) noexcept;
    void PaintHorizontalGlyphAssembly(const RenderStyle &style, PaintInfo &info,
                                      const LayoutPoint &paintOffset) noexcept;
  };
}
