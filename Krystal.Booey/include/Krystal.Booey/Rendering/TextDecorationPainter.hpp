#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Rendering/PaintPhase.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/InlineTextBoxStyle.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationLine.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextShadow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  class FontCascade;
  class GraphicsContext;
  class RenderObject;
  class RenderStyle;
  class TextRun;

  /// @brief Paints text decorations (underlines, overlines and line-throughs) and their shadows: underlines
  /// and overlines under the text (skipping its ink), line-throughs over it.
  class TextDecorationPainter
  {
  private:
    GraphicsContext &_context;
    bool _isPrinting {false};
    WritingMode _writingMode;
    const style::TextShadows &_shadow;
    const FontCascade &_font;

  public:
    struct Styles
    {
      struct DecorationStyleAndColor
      {
        Color DecorationColor;
        TextDecorationStyle DecorationStyle {TextDecorationStyle::Solid};
      };
      DecorationStyleAndColor Underline;
      DecorationStyleAndColor Overline;
      DecorationStyleAndColor Linethrough;

      KRYS_NODISCARD bool operator==(const Styles &other) const noexcept;
    };
    struct BackgroundDecorationGeometry
    {
      FloatPoint TextOrigin;
      FloatPoint BoxOrigin;
      float TextBoxWidth {0.f};
      float TextDecorationThickness {0.f};
      float UnderlineOffset {0.f};
      float OverlineOffset {0.f};
      float LinethroughCenter {0.f};
      float ClippingOffset {0.f};
      WavyStrokeParameters WavyParameters;
    };
    struct ForegroundDecorationGeometry
    {
      FloatPoint BoxOrigin;
      float TextBoxWidth {0.f};
      float TextDecorationThickness {0.f};
      float LinethroughCenter {0.f};
      WavyStrokeParameters WavyParameters;
    };

    TextDecorationPainter(GraphicsContext &context, const FontCascade &font, const style::TextShadows &shadow,
                          bool isPrinting, WritingMode writingMode) noexcept;

    /// @brief Paints the underlines and overlines of @p textRun (and the shadows of all its decorations).
    void PaintBackgroundDecorations(const RenderStyle &style, const TextRun &textRun,
                                    const BackgroundDecorationGeometry &decorationGeometry,
                                    style::TextDecorationLine decorationType,
                                    const Styles &decorationStyle) noexcept;
    /// @brief Paints the line-through, over the text.
    void PaintForegroundDecorations(const ForegroundDecorationGeometry &foregroundDecorationGeometry,
                                    const Styles &decorationStyle) noexcept;

    /// @brief The colour decorations are painted in with @p style.
    KRYS_NODISCARD static Color DecorationColor(const RenderStyle &style,
                                                EnumSet<PaintBehavior> paintBehavior = {}) noexcept;
    /// @brief The colour and style of each of @p requestedDecorations as they apply to @p renderer: from it
    /// and the ancestors they propagate from.
    /// @param firstLineStyle Whether the renderer is on the first line, so ancestors' first line styles
    /// override their styles.
    /// FIXME: Decorations from pseudo-elements' styles (spelling and grammar errors, highlights) come with
    /// marked text.
    KRYS_NODISCARD static Styles StylesForRenderer(const RenderObject &renderer,
                                                   style::TextDecorationLine requestedDecorations,
                                                   bool firstLineStyle = false,
                                                   EnumSet<PaintBehavior> paintBehavior = {}) noexcept;
    /// @brief The decorations @p style has a colour for.
    KRYS_NODISCARD static style::TextDecorationLine
      TextDecorationsInEffectForStyle(const TextDecorationPainter::Styles &style) noexcept;

  private:
    void PaintLineThrough(const ForegroundDecorationGeometry &foregroundDecorationGeometry,
                          const Color &color, const Styles &decorationStyle) noexcept;
  };
}
