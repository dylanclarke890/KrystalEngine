#pragma once

#include "Krystal.Booey/Graphics/GraphicsContextSwitcher.hpp"

namespace krys::boo
{
  class FilterResults;
  class ImageBuffer;

  /// @brief Draws the source into an image buffer, then draws the buffer into the destination context, with
  /// the filter applied to it when there is one.
  class ImageBufferContextSwitcher final : public GraphicsContextSwitcher
  {
  private:
    RefPtr<ImageBuffer> _sourceImage;
    FloatRect _sourceImageRect;
    FilterResults *_results {nullptr};

  public:
    ImageBufferContextSwitcher(GraphicsContext &destinationContext, const FloatRect &sourceImageRect,
                               RefPtr<Filter> &&filter = nullptr, FilterResults *results = nullptr) noexcept;
    ~ImageBufferContextSwitcher() noexcept override;

  private:
    KRYS_NODISCARD GraphicsContext *
      DrawingContext(GraphicsContext &destinationContext) const noexcept override;

    KRYS_NODISCARD bool HasSourceImage() const noexcept override
    {
      return static_cast<bool>(_sourceImage);
    }

    void BeginClipAndDrawSourceImage(GraphicsContext &destinationContext, const FloatRect &repaintRect,
                                     const FloatRect &clipRect) noexcept override;
    void EndClipAndDrawSourceImage(GraphicsContext &destinationContext) noexcept override;

    void BeginDrawSourceImage(GraphicsContext &, float = 1.f) noexcept override
    {
    }
    void EndDrawSourceImage(GraphicsContext &destinationContext) noexcept override;
  };
}
