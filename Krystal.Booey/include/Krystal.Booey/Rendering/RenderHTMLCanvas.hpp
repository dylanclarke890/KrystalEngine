#pragma once

#include "Krystal.Booey/Rendering/RenderReplaced.hpp"

namespace krys::boo::html
{
  class HTMLCanvasElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<canvas>`: replaced content at the canvas's size, which its context draws.
  class RenderHTMLCanvas final : public RenderReplaced
  {
    KRYS_NON_COPY_MOVABLE(RenderHTMLCanvas);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderHTMLCanvas);

  public:
    RenderHTMLCanvas(html::HTMLCanvasElement &element, RenderStyle &&style) noexcept;
    ~RenderHTMLCanvas() noexcept override;

    KRYS_NODISCARD html::HTMLCanvasElement &CanvasElement() const noexcept;

    /// @brief Follows the canvas's size, zoomed, as the intrinsic size.
    void CanvasSizeChanged() noexcept;

  private:
    KRYS_NODISCARD bool RequiresLayer() const noexcept override;
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderHTMLCanvas";
    }
    void PaintReplaced(PaintInfo &, const LayoutPoint &) noexcept override;
    void IntrinsicSizeChanged() noexcept override
    {
      CanvasSizeChanged();
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderHTMLCanvas)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderHTMLCanvas();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
