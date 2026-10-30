#pragma once

#include "Krystal.Booey/Graphics/ImagePaintingOptions.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class GraphicsContext;
  class LayoutRect;
  class LayoutSize;
  class RenderElement;
  class RenderStyle;

  namespace style
  {
    struct BorderImage;
    struct MaskBorder;
  }

  /// @brief Paints a nine-piece image (`border-image` or `mask-border`): the image's corners, sides and
  /// middle sliced, scaled and tiled into the border image area.
  class NinePieceImagePainter
  {
  public:
    static void Paint(const style::BorderImage &, GraphicsContext &, const RenderElement *,
                      const RenderStyle &, const LayoutRect &destination, const LayoutSize &source,
                      float deviceScaleFactor, ImagePaintingOptions) noexcept;
    static void Paint(const style::MaskBorder &, GraphicsContext &, const RenderElement *,
                      const RenderStyle &, const LayoutRect &destination, const LayoutSize &source,
                      float deviceScaleFactor, ImagePaintingOptions) noexcept;
  };
}
