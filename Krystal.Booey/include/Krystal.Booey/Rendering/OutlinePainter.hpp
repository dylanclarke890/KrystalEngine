#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Style/Values/Borders/BorderRadius.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class RenderBlock;
  class RenderBlockFlow;
  class RenderBox;
  class RenderElement;
  class RenderInline;
  class RenderLayerModelObject;
  struct PaintInfo;

  /// @brief Paints a box's outline (`outline`), or its focus ring for `outline-style: auto`.
  /// FIXME: List box and SVG focus rings, and link annotations for PDFs come with controls, SVG and printing.
  class OutlinePainter
  {
  private:
    const PaintInfo &_paintInfo;

  public:
    explicit OutlinePainter(const PaintInfo &) noexcept;

    void PaintOutline(const RenderElement &, const LayoutRect &paintRect) const noexcept;
    /// @brief Paints an inline's outline around its line boxes, shrink-wrapped into one shape where they
    /// touch.
    void PaintOutline(const RenderInline &, const LayoutPoint &paintOffset) const noexcept;

    KRYS_NODISCARD static List<LayoutRect>
      CollectFocusRingRects(const RenderElement &, const LayoutPoint &additionalOffset,
                            const RenderLayerModelObject *paintContainer) noexcept;

    /// @brief One path outlining @p rects (an inline's line boxes) with the box's corner radii on its first
    /// and last lines, or an empty path when they don't join into one shape.
    KRYS_NODISCARD static Path PathWithShrinkWrappedRects(const List<FloatRect> &rects,
                                                          const style::BorderRadius &radii,
                                                          float outlineOffset, WritingMode writingMode,
                                                          float deviceScaleFactor) noexcept;

  private:
    void PaintOutlineWithLineRects(const RenderInline &, const LayoutPoint &paintOffset,
                                   const List<LayoutRect> &lineRects) const noexcept;
    void PaintFocusRing(const RenderElement &, const List<LayoutRect> &) const noexcept;

    static void CollectFocusRingRects(const RenderElement &, List<LayoutRect> &,
                                      const LayoutPoint &additionalOffset,
                                      const RenderLayerModelObject *paintContainer) noexcept;
    static void CollectFocusRingRectsForInline(const RenderInline &, List<LayoutRect> &, const LayoutPoint &,
                                               const RenderLayerModelObject *) noexcept;
    static void CollectFocusRingRectsForInlineChildren(const RenderBlockFlow &, List<LayoutRect> &,
                                                       const LayoutPoint &,
                                                       const RenderLayerModelObject *) noexcept;
    static bool CollectFocusRingRectsForBlock(const RenderBlock &, List<LayoutRect> &, const LayoutPoint &,
                                              const RenderLayerModelObject *) noexcept;
    static void CollectFocusRingRectsForChildBox(const RenderBox &, List<LayoutRect> &, const LayoutPoint &,
                                                 const RenderLayerModelObject *) noexcept;
  };
}
