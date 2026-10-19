#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Rendering/PaintPhase.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/TextDecoration/TextDecorationLine.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  class RenderObject;
  class RenderStyle;

  /// @brief Paints text decorations (underlines, overlines and line-throughs) and their shadows.
  /// FIXME: Painting the decorations (`paintBackgroundDecorations`, `paintForegroundDecorations`, the wavy
  /// stroke and skip-ink) is still to come; the decorations' styles are resolved already, for marked text.
  class TextDecorationPainter
  {
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
  };
}
