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
  }

  /// @brief Paints a nine-piece image (`border-image`): the image's corners, sides and middle sliced, scaled
  /// and tiled into the border image area.
  /// FIXME: `mask-border` comes with masks.
  class NinePieceImagePainter
  {
  public:
    static void Paint(const style::BorderImage &, GraphicsContext &, const RenderElement *,
                      const RenderStyle &, const LayoutRect &destination, const LayoutSize &source,
                      float deviceScaleFactor, ImagePaintingOptions) noexcept;
  };
}
