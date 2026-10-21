#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterStyle.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextSwitcher.hpp"

namespace krys::boo
{
  /// @brief Applies a filter through the graphics context: the source is drawn in a transparency layer per
  /// filter style, each composited with its style.
  class TransparencyLayerContextSwitcher final : public GraphicsContextSwitcher
  {
  private:
    FilterStyleVector _filterStyles;

  public:
    TransparencyLayerContextSwitcher(GraphicsContext &destinationContext, const FloatRect &sourceImageRect,
                                     RefPtr<Filter> &&filter) noexcept;

    void BeginClipAndDrawSourceImage(GraphicsContext &destinationContext, const FloatRect &repaintRect,
                                     const FloatRect &clipRect) noexcept override;
    void EndClipAndDrawSourceImage(GraphicsContext &destinationContext) noexcept override
    {
      EndDrawSourceImage(destinationContext);
    }

    void BeginDrawSourceImage(GraphicsContext &destinationContext, float opacity = 1.f) noexcept override;
    void EndDrawSourceImage(GraphicsContext &destinationContext) noexcept override;
  };
}
