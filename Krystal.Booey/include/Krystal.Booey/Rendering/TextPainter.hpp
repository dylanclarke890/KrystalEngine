#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/RotationDirection.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/TextPaintStyle.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextShadow.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  class FontCascade;
  class GraphicsContext;
  class RenderStyle;
  class TextRun;

  /// @brief The transform turning @p boxRect a quarter turn in @p direction about its top left corner's
  /// diagonal, for vertical text.
  KRYS_NODISCARD inline AffineTransform Rotation(const FloatRect &boxRect,
                                                 RotationDirection direction) noexcept
  {
    return direction == RotationDirection::Clockwise
             ? AffineTransform(0, 1, -1, 0, boxRect.X() + boxRect.MaxY(), boxRect.MaxY() - boxRect.X())
             : AffineTransform(0, -1, 1, 0, boxRect.X() - boxRect.MaxY(), boxRect.X() + boxRect.MaxY());
  }

  /// @brief Paints a range of a text run: its shadows, the text, and its emphasis marks.
  /// FIXME: Cached glyph display lists (`setGlyphDisplayListIfNeeded`, `GlyphDisplayListCache`) and combined
  /// text (`text-combine-upright`, `RenderCombineText`) are still to come.
  class TextPainter
  {
  private:
    GraphicsContext &_context;
    const FontCascade &_font;
    const RenderStyle &_renderStyle;
    TextPaintStyle _style;
    StringAtom _emphasisMark;
    const style::TextShadows &_shadow;
    float _emphasisMarkOffset {0};

  public:
    TextPainter(GraphicsContext &context, const FontCascade &font, const RenderStyle &renderStyle,
                const TextPaintStyle &textPaintStyle, const style::TextShadows &shadow,
                const StringAtom &emphasisMark, float emphasisMarkOffset) noexcept;

    /// @brief Paints @p textRun from byte offset @p start to @p end.
    /// @param boxRect The rect of the whole text box.
    /// @param textOrigin Where the whole text box's text starts, on its baseline.
    void PaintRange(const TextRun &textRun, const FloatRect &boxRect, const FloatPoint &textOrigin,
                    uint32 start, uint32 end) noexcept;

  private:
    void PaintTextOrEmphasisMarks(const FontCascade &font, const TextRun &textRun,
                                  const StringAtom &emphasisMark, float emphasisMarkOffset,
                                  const FloatPoint &textOrigin, uint32 startOffset,
                                  uint32 endOffset) noexcept;
    void PaintTextWithShadows(const style::TextShadows *shadows, const FontCascade &font,
                              const TextRun &textRun, const FloatRect &boxRect, const FloatPoint &textOrigin,
                              uint32 startOffset, uint32 endOffset, const StringAtom &emphasisMark,
                              float emphasisMarkOffset, bool stroked) noexcept;
    void PaintTextAndEmphasisMarksIfNeeded(const TextRun &textRun, const FloatRect &boxRect,
                                           const FloatPoint &textOrigin, uint32 startOffset, uint32 endOffset,
                                           const TextPaintStyle &paintStyle,
                                           const style::TextShadows &shadow) noexcept;
  };

  /// @brief Sets a context up to draw one shadow of some text for as long as it lives: the shadow alone
  /// (clipped to where it falls, the text drawn far off it) or, for the last shadow, the shadow and the text
  /// together.
  class ShadowApplier
  {
    KRYS_NON_COPY_MOVABLE(ShadowApplier);

  private:
    FloatSize _extraOffset;
    GraphicsContext &_context;
    const style::TextShadow *_shadow;
    bool _onlyDrawsShadow : 1;
    bool _avoidDrawingShadow : 1;
    bool _nothingToDraw : 1;
    bool _didSaveContext : 1;

  public:
    ShadowApplier(const RenderStyle &renderStyle, GraphicsContext &context, const style::TextShadow *shadow,
                  const FloatRect &textRect, bool isLastShadowIteration,
                  bool lastShadowIterationShouldDrawText = true, bool opaque = false,
                  bool ignoreWritingMode = false) noexcept;
    ~ShadowApplier() noexcept;

    /// @brief How far the text is moved to draw only its shadow.
    KRYS_NODISCARD FloatSize ExtraOffset() const noexcept
    {
      return _extraOffset;
    }
    KRYS_NODISCARD bool NothingToDraw() const noexcept
    {
      return _nothingToDraw;
    }
    KRYS_NODISCARD bool DidSaveContext() const noexcept
    {
      return _didSaveContext;
    }

  private:
    KRYS_NODISCARD bool ShadowIsCompletelyCoveredByText(bool textIsOpaque) const noexcept;
  };
}
