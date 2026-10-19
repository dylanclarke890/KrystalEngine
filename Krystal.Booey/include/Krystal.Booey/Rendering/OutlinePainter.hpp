#pragma once

#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderBox;
  class RenderElement;
  class RenderLayerModelObject;
  struct PaintInfo;

  /// @brief Paints a box's outline (`outline`), or its focus ring for `outline-style: auto`.
  /// FIXME: Inline outlines (around their line boxes, shrink-wrapped with `pathWithShrinkWrappedRects`),
  /// list box and SVG focus rings, and link annotations for PDFs come with inline painting, controls, SVG and
  /// printing.
  class OutlinePainter
  {
  private:
    const PaintInfo &_paintInfo;

  public:
    explicit OutlinePainter(const PaintInfo &) noexcept;

    void PaintOutline(const RenderElement &, const LayoutRect &paintRect) const noexcept;

    KRYS_NODISCARD static List<LayoutRect>
      CollectFocusRingRects(const RenderElement &, const LayoutPoint &additionalOffset,
                            const RenderLayerModelObject *paintContainer) noexcept;

  private:
    void PaintFocusRing(const RenderElement &, const List<LayoutRect> &) const noexcept;

    static void CollectFocusRingRects(const RenderElement &, List<LayoutRect> &,
                                      const LayoutPoint &additionalOffset,
                                      const RenderLayerModelObject *paintContainer) noexcept;
    static bool CollectFocusRingRectsForBlock(const RenderBlock &, List<LayoutRect> &, const LayoutPoint &,
                                              const RenderLayerModelObject *) noexcept;
    static void CollectFocusRingRectsForChildBox(const RenderBox &, List<LayoutRect> &, const LayoutPoint &,
                                                 const RenderLayerModelObject *) noexcept;
  };
}
