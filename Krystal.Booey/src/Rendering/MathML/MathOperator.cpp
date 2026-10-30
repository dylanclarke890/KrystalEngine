#include "Krystal.Booey/Rendering/MathML/MathOperator.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/GlyphBufferMembers.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    constexpr char32 RadicalOperator = 0x221A;
    constexpr uint32 MaximumExtensionCount = 128;

    KRYS_NODISCARD FloatRect BoundsForGlyph(const GlyphData &data) noexcept
    {
      return data.Font ? data.Font->BoundsForGlyph(data.Glyph) : FloatRect();
    }

    KRYS_NODISCARD float HeightForGlyph(const GlyphData &data) noexcept
    {
      return BoundsForGlyph(data).Height();
    }

    void GetAscentAndDescentForGlyph(const GlyphData &data, LayoutUnit &ascent, LayoutUnit &descent) noexcept
    {
      FloatRect bounds = BoundsForGlyph(data);
      ascent = LayoutUnit(-bounds.Y());
      descent = LayoutUnit(bounds.MaxY());
    }

    KRYS_NODISCARD float AdvanceWidthForGlyph(const GlyphData &data) noexcept
    {
      return data.Font ? data.Font->WidthForGlyph(data.Glyph) : 0;
    }

    // FIXME: This hardcoded data can be removed when OpenType MATH font are widely available
    // (http://wkbug/156837).
    struct StretchyCharacter
    {
      char32 Character;
      char16 TopChar;
      char16 ExtensionChar;
      char16 BottomChar;
      char16 MiddleChar;
    };

    constexpr Array<StretchyCharacter, 14> StretchyCharacters {{
      {0x28, 0x239b, 0x239c, 0x239d, 0x0},    // left parenthesis
      {0x29, 0x239e, 0x239f, 0x23a0, 0x0},    // right parenthesis
      {0x5b, 0x23a1, 0x23a2, 0x23a3, 0x0},    // left square bracket
      {0x5d, 0x23a4, 0x23a5, 0x23a6, 0x0},    // right square bracket
      {0x7b, 0x23a7, 0x23aa, 0x23a9, 0x23a8}, // left curly bracket
      {0x7d, 0x23ab, 0x23aa, 0x23ad, 0x23ac}, // right curly bracket
      {0x2308, 0x23a1, 0x23a2, 0x23a2, 0x0},  // left ceiling
      {0x2309, 0x23a4, 0x23a5, 0x23a5, 0x0},  // right ceiling
      {0x230a, 0x23a2, 0x23a2, 0x23a3, 0x0},  // left floor
      {0x230b, 0x23a5, 0x23a5, 0x23a6, 0x0},  // right floor
      {0x7c, 0x7c, 0x7c, 0x7c, 0x0},          // vertical bar
      {0x2016, 0x2016, 0x2016, 0x2016, 0x0},  // double vertical line
      {0x2225, 0x2225, 0x2225, 0x2225, 0x0},  // parallel to
      {0x222b, 0x2320, 0x23ae, 0x2321, 0x0},  // integral sign
    }};

    KRYS_NODISCARD GlyphData GlyphDataForCodePointOrFallbackGlyph(const RenderStyle &style, char32 codePoint,
                                                                  Glyph fallbackGlyph) noexcept
    {
      if (codePoint)
      {
        return style.GetFontCascade().GlyphDataForCharacter(codePoint, false);
      }

      GlyphData fallback;

      if (fallbackGlyph)
      {
        fallback.Glyph = fallbackGlyph;
        fallback.Font = WeakPtr<const Font>::WithRef(style.GetFontCascade().PrimaryFont().get());
      }

      return fallback;
    }

    // The MathML specification recommends avoiding combining characters.
    // See https://www.w3.org/TR/MathML/chapter7.html#chars.comb-chars
    // However, many math fonts do not provide constructions for the non-combining equivalent.
    constexpr Array<Array<char32, 3>, 6> CharacterFallback {{
      {0x005E, 0x0302, 0},      // CIRCUMFLEX ACCENT
      {0x005F, 0x0332, 0},      // LOW LINE
      {0x007E, 0x0303, 0},      // TILDE
      {0x00AF, 0x0304, 0x0305}, // MACRON
      {0x02C6, 0x0302, 0},      // MODIFIER LETTER CIRCUMFLEX ACCENT
      {0x02C7, 0x030C, 0},      // CARON
    }};
  }

  void MathOperator::SetOperator(const RenderStyle &style, char32 baseCharacter, Type operatorType) noexcept
  {
    _baseCharacter = baseCharacter;
    _operatorType = operatorType;
    Reset(style);
  }

  void MathOperator::Reset(const RenderStyle &style) noexcept
  {
    _stretchType = StretchType::Unstretched;
    _maxPreferredWidth = 0;
    _width = 0;
    _ascent = 0;
    _descent = 0;
    _italicCorrection = 0;
    _radicalVerticalScale = 1;

    // We use the base size for the calculation of the preferred width.
    GlyphData baseGlyph;
    if (!GetBaseGlyph(style, baseGlyph))
    {
      return;
    }
    _maxPreferredWidth = _width = LayoutUnit(AdvanceWidthForGlyph(baseGlyph));
    GetAscentAndDescentForGlyph(baseGlyph, _ascent, _descent);

    if (_operatorType == Type::VerticalOperator)
    {
      // We also take into account the width of larger sizes for the calculation of the preferred width.
      CalculateStretchyData(style, true);
    }
    else if (_operatorType == Type::DisplayOperator)
    {
      // We can directly select the size variant and determine the final metrics.
      CalculateDisplayStyleLargeOperator(style);
    }
  }

  LayoutUnit MathOperator::StretchSize() const noexcept
  {
    krys_debug_assert(_operatorType == Type::VerticalOperator || _operatorType == Type::HorizontalOperator);
    return _operatorType == Type::VerticalOperator ? _ascent + _descent : _width;
  }

  bool MathOperator::GetGlyph(const RenderStyle &style, char32 character, GlyphData &glyph) const noexcept
  {
    const auto &fontCascade = style.GetFontCascade();
    glyph = fontCascade.GlyphDataForCharacter(character, style.GetWritingMode().IsBidiRTL());
    return glyph.Font && glyph.Font.get() == fontCascade.PrimaryFont().get();
  }

  void MathOperator::SetSizeVariant(const GlyphData &sizeVariant) noexcept
  {
    krys_debug_assert(sizeVariant.Font);
    krys_debug_assert(sizeVariant.Font->MathData());
    _stretchType = StretchType::SizeVariant;
    _variantGlyph = sizeVariant.Glyph;
    _width = LayoutUnit(AdvanceWidthForGlyph(sizeVariant));
    GetAscentAndDescentForGlyph(sizeVariant, _ascent, _descent);
  }

  void MathOperator::SetGlyphAssembly(const RenderStyle &style,
                                      const GlyphAssemblyData &assemblyData) noexcept
  {
    krys_debug_assert(_operatorType == Type::VerticalOperator || _operatorType == Type::HorizontalOperator);
    _stretchType = StretchType::GlyphAssembly;
    _assembly = assemblyData;

    auto topOrRight = GlyphDataForCodePointOrFallbackGlyph(style, _assembly.TopOrRightCodePoint,
                                                           _assembly.TopOrRightFallbackGlyph);
    auto extension = GlyphDataForCodePointOrFallbackGlyph(style, _assembly.ExtensionCodePoint,
                                                          _assembly.ExtensionFallbackGlyph);
    auto middle =
      GlyphDataForCodePointOrFallbackGlyph(style, _assembly.MiddleCodePoint, _assembly.MiddleFallbackGlyph);
    auto bottomOrLeft = GlyphDataForCodePointOrFallbackGlyph(style, _assembly.BottomOrLeftCodePoint,
                                                             _assembly.BottomOrLeftFallbackGlyph);

    if (_operatorType == Type::VerticalOperator)
    {
      _width = 0;
      _width = std::max(_width, LayoutUnit(AdvanceWidthForGlyph(topOrRight)));
      _width = std::max(_width, LayoutUnit(AdvanceWidthForGlyph(extension)));
      _width = std::max(_width, LayoutUnit(AdvanceWidthForGlyph(bottomOrLeft)));
      _width = std::max(_width, LayoutUnit(AdvanceWidthForGlyph(middle)));
    }
    else
    {
      _ascent = 0;
      _descent = 0;
      LayoutUnit ascent, descent;
      for (const auto *part : {&bottomOrLeft, &extension, &topOrRight, &middle})
      {
        GetAscentAndDescentForGlyph(*part, ascent, descent);
        _ascent = std::max(_ascent, ascent);
        _descent = std::max(_descent, descent);
      }
    }
  }

  void MathOperator::GetMathVariantsWithFallback(
    const RenderStyle &style, bool isVertical, List<Glyph> &sizeVariants,
    List<OpenTypeMathData::AssemblyPart> &assemblyParts) const noexcept
  {
    // In general, we first try and find contruction for the base glyph.
    GlyphData baseGlyph;
    if (!GetBaseGlyph(style, baseGlyph) || !baseGlyph.Font->MathData())
    {
      return;
    }
    baseGlyph.Font->MathData()->GetMathVariants(baseGlyph.Glyph, isVertical, sizeVariants, assemblyParts);
    if (!sizeVariants.empty() || !assemblyParts.empty())
    {
      return;
    }

    // Otherwise, we try and find fallback constructions using similar characters.
    for (const auto &fallbacks : CharacterFallback)
    {
      uint32 j = 0;
      if (fallbacks[j] == _baseCharacter)
      {
        for (j++; j < fallbacks.size() && fallbacks[j]; j++)
        {
          GlyphData glyphData;
          if (!GetGlyph(style, fallbacks[j], glyphData))
          {
            continue;
          }
          glyphData.Font->MathData()->GetMathVariants(glyphData.Glyph, isVertical, sizeVariants,
                                                      assemblyParts);
          if (!sizeVariants.empty() || !assemblyParts.empty())
          {
            return;
          }
        }
        break;
      }
    }
  }

  void MathOperator::CalculateDisplayStyleLargeOperator(const RenderStyle &style) noexcept
  {
    krys_debug_assert(_operatorType == Type::DisplayOperator);

    GlyphData baseGlyph;
    if (!GetBaseGlyph(style, baseGlyph) || !baseGlyph.Font->MathData())
    {
      return;
    }

    const auto *mathData = baseGlyph.Font->MathData();
    float displayOperatorMinHeight =
      mathData->GetMathConstant(*baseGlyph.Font, OpenTypeMathData::MathConstant::DisplayOperatorMinHeight);

    List<Glyph> sizeVariants;
    List<OpenTypeMathData::AssemblyPart> assemblyParts;
    mathData->GetMathVariants(baseGlyph.Glyph, true, sizeVariants, assemblyParts);

    // We choose the first size variant that is larger than the expected displayOperatorMinHeight and
    // otherwise fallback to the largest variant.
    for (auto sizeVariant : sizeVariants)
    {
      GlyphData glyphData(sizeVariant, baseGlyph.Font.get());
      SetSizeVariant(glyphData);
      _maxPreferredWidth = _width;
      _italicCorrection = LayoutUnit(mathData->GetItalicCorrection(*glyphData.Font, glyphData.Glyph));
      if (HeightForGlyph(glyphData) >= displayOperatorMinHeight)
      {
        break;
      }
    }
  }

  bool MathOperator::CalculateGlyphAssemblyFallback(const List<OpenTypeMathData::AssemblyPart> &assemblyParts,
                                                    GlyphAssemblyData &assemblyData) const noexcept
  {
    // The structure of the Open Type Math table is a bit more general than the one currently used by the
    // MathOperator code, so we try to fallback in a reasonable way.
    // FIXME: MathOperator should support the most general format
    // (https://bugs.webkit.org/show_bug.cgi?id=130327).
    // We use the approach of the copyComponents function in
    // github.com/mathjax/MathJax-dev/blob/master/fonts/OpenTypeMath/fontUtil.py

    // We count the number of non extender pieces.
    int32 nonExtenderCount = 0;
    for (const auto &part : assemblyParts)
    {
      if (!part.IsExtender)
      {
        nonExtenderCount++;
      }
    }
    if (nonExtenderCount > 3)
    {
      return false; // This is not supported: there are too many pieces.
    }

    // We now browse the list of pieces from left to right for horizontal operators and from bottom to top for
    // vertical operators.
    enum class PartType : uint8
    {
      Start,
      ExtenderBetweenStartAndMiddle,
      Middle,
      ExtenderBetweenMiddleAndEnd,
      End,
      None
    };
    PartType expectedPartType = PartType::Start;
    assemblyData.ExtensionCodePoint = 0;
    assemblyData.ExtensionFallbackGlyph = 0;
    assemblyData.MiddleCodePoint = 0;
    assemblyData.MiddleFallbackGlyph = 0;
    for (const auto &part : assemblyParts)
    {
      if (nonExtenderCount < 3)
      {
        // If we only have at most two non-extenders then we skip the middle glyph.
        if (expectedPartType == PartType::ExtenderBetweenStartAndMiddle)
        {
          expectedPartType = PartType::ExtenderBetweenMiddleAndEnd;
        }
        else if (expectedPartType == PartType::Middle)
        {
          expectedPartType = PartType::End;
        }
      }
      if (part.IsExtender)
      {
        if (!assemblyData.ExtensionFallbackGlyph)
        {
          assemblyData.ExtensionFallbackGlyph = part.Glyph; // We copy the extender part.
        }
        else if (assemblyData.ExtensionFallbackGlyph != part.Glyph)
        {
          return false; // This is not supported: the assembly has different extenders.
        }

        switch (expectedPartType)
        {
          case PartType::Start:
            // We ignore the left/bottom part.
            expectedPartType = PartType::ExtenderBetweenStartAndMiddle;
            continue;
          case PartType::Middle:
            // We ignore the middle part.
            expectedPartType = PartType::ExtenderBetweenMiddleAndEnd;
            continue;
          case PartType::End:
          case PartType::None:
            // This is not supported: we got an unexpected extender.
            return false;
          case PartType::ExtenderBetweenStartAndMiddle:
          case PartType::ExtenderBetweenMiddleAndEnd:
            // We ignore multiple consecutive extenders.
            continue;
        }
      }

      switch (expectedPartType)
      {
        case PartType::Start:
          // We copy the left/bottom part.
          assemblyData.BottomOrLeftFallbackGlyph = part.Glyph;
          assemblyData.BottomOrLeftCodePoint = 0;
          expectedPartType = PartType::ExtenderBetweenStartAndMiddle;
          continue;
        case PartType::ExtenderBetweenStartAndMiddle:
        case PartType::Middle:
          // We copy the middle part.
          assemblyData.MiddleFallbackGlyph = part.Glyph;
          expectedPartType = PartType::ExtenderBetweenMiddleAndEnd;
          continue;
        case PartType::ExtenderBetweenMiddleAndEnd:
        case PartType::End:
          // We copy the right/top part.
          assemblyData.TopOrRightFallbackGlyph = part.Glyph;
          assemblyData.TopOrRightCodePoint = 0;
          expectedPartType = PartType::None;
          continue;
        case PartType::None:
          // This is not supported: we got an unexpected non-extender part.
          return false;
      }
    }

    if (!assemblyData.HasExtension())
    {
      return false; // This is not supported: we always assume that we have an extension glyph.
    }

    // If we don't have top/bottom glyphs, we use the extension glyph.
    if (!assemblyData.TopOrRightCodePoint && !assemblyData.TopOrRightFallbackGlyph)
    {
      assemblyData.TopOrRightFallbackGlyph = assemblyData.ExtensionFallbackGlyph;
    }
    if (!assemblyData.BottomOrLeftCodePoint && !assemblyData.BottomOrLeftFallbackGlyph)
    {
      assemblyData.BottomOrLeftFallbackGlyph = assemblyData.ExtensionFallbackGlyph;
    }

    return true;
  }

  void MathOperator::CalculateStretchyData(const RenderStyle &style, bool calculateMaxPreferredWidth,
                                           LayoutUnit targetSize) noexcept
  {
    krys_debug_assert(_operatorType == Type::VerticalOperator || _operatorType == Type::HorizontalOperator);
    krys_debug_assert(!calculateMaxPreferredWidth || _operatorType == Type::VerticalOperator);
    bool isVertical = _operatorType == Type::VerticalOperator;

    GlyphData baseGlyph;
    if (!GetBaseGlyph(style, baseGlyph))
    {
      return;
    }

    if (!calculateMaxPreferredWidth)
    {
      // We do not stretch if the base glyph is large enough.
      float baseSize = isVertical ? HeightForGlyph(baseGlyph) : AdvanceWidthForGlyph(baseGlyph);
      if (targetSize <= baseSize)
      {
        return;
      }
    }

    GlyphAssemblyData assemblyData;
    if (baseGlyph.Font->MathData())
    {
      List<Glyph> sizeVariants;
      List<OpenTypeMathData::AssemblyPart> assemblyParts;
      GetMathVariantsWithFallback(style, isVertical, sizeVariants, assemblyParts);
      // We verify the size variants.
      for (auto sizeVariant : sizeVariants)
      {
        GlyphData glyphData(sizeVariant, baseGlyph.Font.get());
        if (calculateMaxPreferredWidth)
        {
          _maxPreferredWidth = std::max(_maxPreferredWidth, LayoutUnit(AdvanceWidthForGlyph(glyphData)));
        }
        else
        {
          SetSizeVariant(glyphData);
          LayoutUnit size {isVertical ? HeightForGlyph(glyphData) : AdvanceWidthForGlyph(glyphData)};
          if (size >= targetSize)
          {
            return;
          }
        }
      }

      // We verify if there is a construction.
      if (!CalculateGlyphAssemblyFallback(assemblyParts, assemblyData))
      {
        return;
      }
    }
    else
    {
      if (!isVertical)
      {
        return;
      }

      // If the font does not have a MATH table, we fallback to the Unicode-only constructions.
      const StretchyCharacter *stretchyCharacter = nullptr;
      for (const auto &character : StretchyCharacters)
      {
        if (character.Character == _baseCharacter)
        {
          stretchyCharacter = &character;
          break;
        }
      }

      // Unicode contains U+23B7 RADICAL SYMBOL BOTTOM but it is generally not provided by fonts without a
      // MATH table. Moreover, it's not clear what the proper vertical extender or top hook would be. Hence we
      // fallback to scaling the base glyph vertically.
      if (!calculateMaxPreferredWidth && _baseCharacter == RadicalOperator)
      {
        LayoutUnit height = _ascent + _descent;
        if (height > 0 && height < targetSize)
        {
          _radicalVerticalScale = targetSize.ToFloat() / height.ToFloat();
          _ascent *= _radicalVerticalScale;
          _descent *= _radicalVerticalScale;
        }
        return;
      }

      // If we didn't find a stretchy character set for this character, we don't know how to stretch it.
      if (!stretchyCharacter)
      {
        return;
      }

      // We convert the list of Unicode characters into a list of glyph data.
      assemblyData.TopOrRightCodePoint = stretchyCharacter->TopChar;
      assemblyData.ExtensionCodePoint = stretchyCharacter->ExtensionChar;
      assemblyData.BottomOrLeftCodePoint = stretchyCharacter->BottomChar;
      assemblyData.MiddleCodePoint = stretchyCharacter->MiddleChar;
    }

    auto topOrRight = GlyphDataForCodePointOrFallbackGlyph(style, assemblyData.TopOrRightCodePoint,
                                                           assemblyData.TopOrRightFallbackGlyph);
    auto extension = GlyphDataForCodePointOrFallbackGlyph(style, assemblyData.ExtensionCodePoint,
                                                          assemblyData.ExtensionFallbackGlyph);
    auto middle = GlyphDataForCodePointOrFallbackGlyph(style, assemblyData.MiddleCodePoint,
                                                       assemblyData.MiddleFallbackGlyph);
    auto bottomOrLeft = GlyphDataForCodePointOrFallbackGlyph(style, assemblyData.BottomOrLeftCodePoint,
                                                             assemblyData.BottomOrLeftFallbackGlyph);

    // If we are measuring the maximum width, verify each component.
    if (calculateMaxPreferredWidth)
    {
      for (const auto *part : {&topOrRight, &extension, &middle, &bottomOrLeft})
      {
        _maxPreferredWidth = std::max(_maxPreferredWidth, LayoutUnit(AdvanceWidthForGlyph(*part)));
      }
      return;
    }

    // We ensure that the size is large enough to avoid glyph overlaps.
    float minSize = isVertical
                      ? HeightForGlyph(topOrRight) + HeightForGlyph(middle) + HeightForGlyph(bottomOrLeft)
                      : AdvanceWidthForGlyph(bottomOrLeft) + AdvanceWidthForGlyph(middle)
                          + AdvanceWidthForGlyph(topOrRight);
    if (minSize > targetSize)
    {
      return;
    }

    SetGlyphAssembly(style, assemblyData);
  }

  void MathOperator::StretchTo(const RenderStyle &style, LayoutUnit targetSize) noexcept
  {
    krys_debug_assert(_operatorType == Type::VerticalOperator || _operatorType == Type::HorizontalOperator);
    CalculateStretchyData(style, false, targetSize);
    if (_stretchType == StretchType::GlyphAssembly)
    {
      if (_operatorType == Type::VerticalOperator)
      {
        _ascent = targetSize;
        _descent = 0;
      }
      else
      {
        _width = targetSize;
      }
    }
  }

  LayoutRect MathOperator::PaintGlyph(const RenderStyle &style, PaintInfo &info, const GlyphData &data,
                                      const LayoutPoint &origin, GlyphPaintTrimming trim) noexcept
  {
    FloatRect glyphBounds = BoundsForGlyph(data);

    LayoutRect glyphPaintRect(origin,
                              LayoutSize(glyphBounds.X() + glyphBounds.Width(), glyphBounds.Height()));
    glyphPaintRect.SetY(origin.Y() + glyphBounds.Y());

    // In order to have glyphs fit snugly with one another we snap the connecting edges to pixel boundaries
    // and trim off one pixel. The pixel trim is to account for fonts that have edge pixels that have less
    // than full coverage. These edge pixels can introduce small seams between connected glyphs.
    FloatRect clipBounds = info.Rect;
    switch (trim)
    {
      case GlyphPaintTrimming::TrimTop:
        glyphPaintRect.ShiftYEdgeTo(glyphPaintRect.Y().Ceil() + 1);
        clipBounds.ShiftYEdgeTo(glyphPaintRect.Y());
        break;
      case GlyphPaintTrimming::TrimBottom:
        glyphPaintRect.ShiftMaxYEdgeTo(glyphPaintRect.MaxY().Floor() - 1);
        clipBounds.ShiftMaxYEdgeTo(glyphPaintRect.MaxY());
        break;
      case GlyphPaintTrimming::TrimTopAndBottom:
        glyphPaintRect.ShiftYEdgeTo(glyphPaintRect.Y().Ceil() + 1);
        glyphPaintRect.ShiftMaxYEdgeTo(glyphPaintRect.MaxY().Floor() - 1);
        clipBounds.ShiftYEdgeTo(glyphPaintRect.Y());
        clipBounds.ShiftMaxYEdgeTo(glyphPaintRect.MaxY());
        break;
      case GlyphPaintTrimming::TrimLeft:
        glyphPaintRect.ShiftXEdgeTo(glyphPaintRect.X().Ceil() + 1);
        clipBounds.ShiftXEdgeTo(glyphPaintRect.X());
        break;
      case GlyphPaintTrimming::TrimRight:
        glyphPaintRect.ShiftMaxXEdgeTo(glyphPaintRect.MaxX().Floor() - 1);
        clipBounds.ShiftMaxXEdgeTo(glyphPaintRect.MaxX());
        break;
      case GlyphPaintTrimming::TrimLeftAndRight:
        glyphPaintRect.ShiftXEdgeTo(glyphPaintRect.X().Ceil() + 1);
        glyphPaintRect.ShiftMaxXEdgeTo(glyphPaintRect.MaxX().Floor() - 1);
        clipBounds.ShiftXEdgeTo(glyphPaintRect.X());
        clipBounds.ShiftMaxXEdgeTo(glyphPaintRect.MaxX());
    }

    // Clipping the enclosing IntRect avoids any potential issues at joined edges.
    GraphicsContextStateSaver stateSaver(info.Context());
    info.Context().Clip(clipBounds);

    // FIXME: If we're just drawing a single glyph, why do we need to compute an advance?
    auto advance = MakeGlyphBufferAdvance(FloatSize(AdvanceWidthForGlyph(data), 0));
    info.Context().DrawGlyphs(*data.Font, Span<const GlyphBufferGlyph>(&data.Glyph, 1),
                              Span<const GlyphBufferAdvance>(&advance, 1), FloatPoint(origin),
                              style.GetFontCascade().GetFontDescription().UsedFontSmoothing());

    return glyphPaintRect;
  }

  void MathOperator::FillWithVerticalExtensionGlyph(const RenderStyle &style, PaintInfo &info,
                                                    const LayoutPoint &from, const LayoutPoint &to) noexcept
  {
    krys_debug_assert(_operatorType == Type::VerticalOperator);
    krys_debug_assert(_stretchType == StretchType::GlyphAssembly);

    auto extension = GlyphDataForCodePointOrFallbackGlyph(style, _assembly.ExtensionCodePoint,
                                                          _assembly.ExtensionFallbackGlyph);

    krys_debug_assert(extension.Font);
    krys_debug_assert(from.Y() <= to.Y());

    // If there is no space for the extension glyph, we don't need to do anything.
    if (from.Y() == to.Y())
    {
      return;
    }

    GraphicsContextStateSaver stateSaver(info.Context());

    FloatRect glyphBounds = BoundsForGlyph(extension);

    // Clipping the extender region here allows us to draw the bottom extender glyph into the
    // regions of the bottom glyph without worrying about overdraw (hairy pixels) and simplifies later
    // clipping.
    LayoutRect clipBounds = info.Rect;
    clipBounds.ShiftYEdgeTo(from.Y());
    clipBounds.ShiftMaxYEdgeTo(to.Y());
    info.Context().Clip(FloatRect(clipBounds));

    // Trimming may remove up to two pixels from the top of the extender glyph, so we move it up by two
    // pixels.
    float offsetToGlyphTop = glyphBounds.Y() + 2;
    LayoutPoint glyphOrigin {from.X(), LayoutUnit(from.Y() - offsetToGlyphTop)};
    FloatRect lastPaintedGlyphRect {FloatPoint(from), FloatSize()};

    // In practice, only small stretch sizes are requested but we limit the number of glyphs to avoid hangs.
    for (uint32 extensionCount = 0;
         lastPaintedGlyphRect.MaxY() < to.Y() && extensionCount < MaximumExtensionCount; extensionCount++)
    {
      lastPaintedGlyphRect =
        FloatRect(PaintGlyph(style, info, extension, glyphOrigin, GlyphPaintTrimming::TrimTopAndBottom));
      glyphOrigin.SetY(glyphOrigin.Y() + LayoutUnit(lastPaintedGlyphRect.Height()));

      // There's a chance that if the font size is small enough the glue glyph has been reduced to an empty
      // rectangle with trimming. In that case we just draw nothing.
      if (lastPaintedGlyphRect.IsEmpty())
      {
        break;
      }
    }
  }

  void MathOperator::FillWithHorizontalExtensionGlyph(const RenderStyle &style, PaintInfo &info,
                                                      const LayoutPoint &from, const LayoutPoint &to) noexcept
  {
    krys_debug_assert(_operatorType == Type::HorizontalOperator);
    krys_debug_assert(_stretchType == StretchType::GlyphAssembly);

    auto extension = GlyphDataForCodePointOrFallbackGlyph(style, _assembly.ExtensionCodePoint,
                                                          _assembly.ExtensionFallbackGlyph);

    krys_debug_assert(extension.Font);
    krys_debug_assert(from.X() <= to.X());
    krys_debug_assert(from.Y() == to.Y());

    // If there is no space for the extension glyph, we don't need to do anything.
    if (from.X() == to.X())
    {
      return;
    }

    GraphicsContextStateSaver stateSaver(info.Context());

    // Clipping the extender region here allows us to draw the bottom extender glyph into the
    // regions of the bottom glyph without worrying about overdraw (hairy pixels) and simplifies later
    // clipping.
    LayoutRect clipBounds = info.Rect;
    clipBounds.ShiftXEdgeTo(from.X());
    clipBounds.ShiftMaxXEdgeTo(to.X());
    info.Context().Clip(FloatRect(clipBounds));

    // Trimming may remove up to two pixels from the left of the extender glyph, so we move it left by two
    // pixels.
    float offsetToGlyphLeft = -2;
    LayoutPoint glyphOrigin {LayoutUnit(from.X() + offsetToGlyphLeft), from.Y()};
    FloatRect lastPaintedGlyphRect {FloatPoint(from), FloatSize()};

    // In practice, only small stretch sizes are requested but we limit the number of glyphs to avoid hangs.
    for (uint32 extensionCount = 0;
         lastPaintedGlyphRect.MaxX() < to.X() && extensionCount < MaximumExtensionCount; extensionCount++)
    {
      lastPaintedGlyphRect =
        FloatRect(PaintGlyph(style, info, extension, glyphOrigin, GlyphPaintTrimming::TrimLeftAndRight));
      glyphOrigin.SetX(glyphOrigin.X() + LayoutUnit(lastPaintedGlyphRect.Width()));

      // There's a chance that if the font size is small enough the glue glyph has been reduced to an empty
      // rectangle with trimming. In that case we just draw nothing.
      if (lastPaintedGlyphRect.IsEmpty())
      {
        break;
      }
    }
  }

  void MathOperator::PaintVerticalGlyphAssembly(const RenderStyle &style, PaintInfo &info,
                                                const LayoutPoint &paintOffset) noexcept
  {
    krys_debug_assert(_operatorType == Type::VerticalOperator);
    krys_debug_assert(_stretchType == StretchType::GlyphAssembly);

    auto topOrRight = GlyphDataForCodePointOrFallbackGlyph(style, _assembly.TopOrRightCodePoint,
                                                           _assembly.TopOrRightFallbackGlyph);
    auto bottomOrLeft = GlyphDataForCodePointOrFallbackGlyph(style, _assembly.BottomOrLeftCodePoint,
                                                             _assembly.BottomOrLeftFallbackGlyph);

    // No font can be found for a code point.
    if (!topOrRight.Font || !bottomOrLeft.Font)
    {
      return;
    }

    // We are positioning the glyphs so that the edge of the tight glyph bounds line up exactly with the edges
    // of our paint box.
    LayoutPoint operatorTopLeft = paintOffset;
    FloatRect topGlyphBounds = BoundsForGlyph(topOrRight);
    LayoutPoint topGlyphOrigin {operatorTopLeft.X(), LayoutUnit(operatorTopLeft.Y() - topGlyphBounds.Y())};
    LayoutRect topGlyphPaintRect =
      PaintGlyph(style, info, topOrRight, topGlyphOrigin, GlyphPaintTrimming::TrimBottom);

    FloatRect bottomGlyphBounds = BoundsForGlyph(bottomOrLeft);
    LayoutPoint bottomGlyphOrigin {
      operatorTopLeft.X(),
      LayoutUnit(operatorTopLeft.Y() + StretchSize() - (bottomGlyphBounds.Height() + bottomGlyphBounds.Y()))};
    LayoutRect bottomGlyphPaintRect =
      PaintGlyph(style, info, bottomOrLeft, bottomGlyphOrigin, GlyphPaintTrimming::TrimTop);

    if (_assembly.HasMiddle())
    {
      auto middle =
        GlyphDataForCodePointOrFallbackGlyph(style, _assembly.MiddleCodePoint, _assembly.MiddleFallbackGlyph);

      // Center the glyph origin between the start and end glyph paint extents. Then shift it half the paint
      // height toward the bottom glyph.
      FloatRect middleGlyphBounds = BoundsForGlyph(middle);
      LayoutPoint middleGlyphOrigin(operatorTopLeft.X(), topGlyphOrigin.Y());
      middleGlyphOrigin.MoveBy(LayoutPoint(0, (bottomGlyphPaintRect.Y() - topGlyphPaintRect.MaxY()) / 2));
      middleGlyphOrigin.MoveBy(LayoutPoint(0, LayoutUnit(middleGlyphBounds.Height() / 2)));

      LayoutRect middleGlyphPaintRect =
        PaintGlyph(style, info, middle, middleGlyphOrigin, GlyphPaintTrimming::TrimTopAndBottom);
      FillWithVerticalExtensionGlyph(style, info, topGlyphPaintRect.MinXMaxYCorner(),
                                     middleGlyphPaintRect.MinXMinYCorner());
      FillWithVerticalExtensionGlyph(style, info, middleGlyphPaintRect.MinXMaxYCorner(),
                                     bottomGlyphPaintRect.MinXMinYCorner());
    }
    else
    {
      FillWithVerticalExtensionGlyph(style, info, topGlyphPaintRect.MinXMaxYCorner(),
                                     bottomGlyphPaintRect.MinXMinYCorner());
    }
  }

  void MathOperator::PaintHorizontalGlyphAssembly(const RenderStyle &style, PaintInfo &info,
                                                  const LayoutPoint &paintOffset) noexcept
  {
    krys_debug_assert(_operatorType == Type::HorizontalOperator);
    krys_debug_assert(_stretchType == StretchType::GlyphAssembly);

    auto topOrRight = GlyphDataForCodePointOrFallbackGlyph(style, _assembly.TopOrRightCodePoint,
                                                           _assembly.TopOrRightFallbackGlyph);
    auto bottomOrLeft = GlyphDataForCodePointOrFallbackGlyph(style, _assembly.BottomOrLeftCodePoint,
                                                             _assembly.BottomOrLeftFallbackGlyph);

    // No font can be found for a code point.
    if (!topOrRight.Font || !bottomOrLeft.Font)
    {
      return;
    }

    // We are positioning the glyphs so that the edge of the tight glyph bounds line up exactly with the edges
    // of our paint box.
    LayoutPoint operatorTopLeft = paintOffset;
    LayoutUnit baselineY = operatorTopLeft.Y() + _ascent;
    LayoutPoint leftGlyphOrigin(operatorTopLeft.X(), baselineY);
    LayoutRect leftGlyphPaintRect =
      PaintGlyph(style, info, bottomOrLeft, leftGlyphOrigin, GlyphPaintTrimming::TrimRight);

    FloatRect rightGlyphBounds = BoundsForGlyph(topOrRight);
    LayoutPoint rightGlyphOrigin {LayoutUnit(operatorTopLeft.X() + StretchSize() - rightGlyphBounds.Width()),
                                  baselineY};
    LayoutRect rightGlyphPaintRect =
      PaintGlyph(style, info, topOrRight, rightGlyphOrigin, GlyphPaintTrimming::TrimLeft);

    if (_assembly.HasMiddle())
    {
      auto middle =
        GlyphDataForCodePointOrFallbackGlyph(style, _assembly.MiddleCodePoint, _assembly.MiddleFallbackGlyph);

      // Center the glyph origin between the start and end glyph paint extents.
      LayoutPoint middleGlyphOrigin(operatorTopLeft.X(), baselineY);
      middleGlyphOrigin.MoveBy(LayoutPoint((rightGlyphPaintRect.X() - leftGlyphPaintRect.MaxX()) / 2, 0));
      LayoutRect middleGlyphPaintRect =
        PaintGlyph(style, info, middle, middleGlyphOrigin, GlyphPaintTrimming::TrimLeftAndRight);
      FillWithHorizontalExtensionGlyph(style, info, LayoutPoint(leftGlyphPaintRect.MaxX(), baselineY),
                                       LayoutPoint(middleGlyphPaintRect.X(), baselineY));
      FillWithHorizontalExtensionGlyph(style, info, LayoutPoint(middleGlyphPaintRect.MaxX(), baselineY),
                                       LayoutPoint(rightGlyphPaintRect.X(), baselineY));
    }
    else
    {
      FillWithHorizontalExtensionGlyph(style, info, LayoutPoint(leftGlyphPaintRect.MaxX(), baselineY),
                                       LayoutPoint(rightGlyphPaintRect.X(), baselineY));
    }
  }

  void MathOperator::Paint(const RenderStyle &style, PaintInfo &info, const LayoutPoint &paintOffset,
                           float deviceScaleFactor) noexcept
  {
    if (info.Context().PaintingDisabled() || info.Phase != PaintPhase::Foreground
        || style.UsedVisibility() != Visibility::Visible)
    {
      return;
    }

    // Make a copy of the PaintInfo because ApplyTransform will modify its rect.
    PaintInfo paintInfo(info);
    GraphicsContextStateSaver stateSaver(paintInfo.Context());
    paintInfo.Context().SetFillColor(style.VisitedDependentColorWithColorFilter(css::PropertyId::Color));

    // For a radical character, we may need some scale transform to stretch it vertically or mirror it.
    if (_baseCharacter == RadicalOperator)
    {
      float radicalHorizontalScale = style.GetWritingMode().IsBidiLTR() ? 1.0f : -1.0f;
      if (radicalHorizontalScale == -1 || _radicalVerticalScale > 1)
      {
        LayoutPoint scaleOrigin = paintOffset;
        scaleOrigin.Move(_width / 2, 0_lu);
        paintInfo.ApplyTransform(AffineTransform()
                                   .Translate(FloatPoint(scaleOrigin))
                                   .Scale(radicalHorizontalScale, _radicalVerticalScale)
                                   .Translate(-FloatPoint(scaleOrigin)));
      }
    }

    if (_stretchType == StretchType::GlyphAssembly)
    {
      if (_operatorType == Type::VerticalOperator)
      {
        PaintVerticalGlyphAssembly(style, info, paintOffset);
      }
      else
      {
        PaintHorizontalGlyphAssembly(style, info, paintOffset);
      }
      return;
    }

    GlyphData glyphData;
    krys_debug_assert(_stretchType == StretchType::Unstretched || _stretchType == StretchType::SizeVariant);
    if (!GetBaseGlyph(style, glyphData))
    {
      return;
    }
    if (_stretchType == StretchType::SizeVariant)
    {
      glyphData.Glyph = _variantGlyph;
    }

    auto operatorTopLeft = paintOffset;
    auto glyphBounds = BoundsForGlyph(glyphData);
    auto operatorOrigin =
      LayoutPoint {operatorTopLeft.X(), LayoutUnit(operatorTopLeft.Y() - glyphBounds.Y())};

    FloatPoint origin(operatorOrigin);
    if (style.GetWritingMode().IsHorizontal())
    {
      origin.SetY(RoundToDevicePixel(operatorOrigin.Y(), deviceScaleFactor));
    }
    else
    {
      origin.SetX(RoundToDevicePixel(operatorOrigin.X(), deviceScaleFactor));
    }

    // FIXME: If we're just drawing a single glyph, why do we need to compute an advance?
    auto advance = MakeGlyphBufferAdvance(FloatSize(AdvanceWidthForGlyph(glyphData), 0));
    paintInfo.Context().DrawGlyphs(*glyphData.Font, Span<const GlyphBufferGlyph>(&glyphData.Glyph, 1),
                                   Span<const GlyphBufferAdvance>(&advance, 1), origin,
                                   style.GetFontCascade().GetFontDescription().UsedFontSmoothing());
  }
}
