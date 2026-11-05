#pragma once

#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class GraphicsContext;
  class Image;
  class RenderElement;
  class RenderStyle;

  /// @brief Chooses how images are interpolated when they're drawn scaled.
  // FIXME: Low quality while an image is resized live (the per-renderer layer sizes and the high quality
  // repaint timer), once there are live resizes.
  class ImageQualityController
  {
  public:
    /// @brief The interpolation `image-rendering` asks for, or nothing for `auto`.
    KRYS_NODISCARD static Maybe<InterpolationQuality>
      InterpolationQualityFromStyle(const RenderStyle &) noexcept;

    /// @brief How an SVG `<image>` drawing @p image is interpolated.
    KRYS_NODISCARD static InterpolationQuality
      ChooseInterpolationQualityForSVG(GraphicsContext &, const RenderElement &, Image &) noexcept;

    /// @brief How @p image is interpolated when @p renderer draws it.
    KRYS_NODISCARD static InterpolationQuality
      ChooseInterpolationQuality(GraphicsContext &, const RenderElement &renderer, Image &image) noexcept;
  };
}
