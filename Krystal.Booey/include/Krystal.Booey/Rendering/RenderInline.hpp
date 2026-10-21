#pragma once

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief An inline box.
  // FIXME: Line boxes, continuations and the geometry come with layout.
  class RenderInline : public RenderBoxModelObject
  {
    KRYS_NON_COPY_MOVABLE(RenderInline);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderInline);

  public:
    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;
    bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                     const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                     HitTestAction hitTestAction) noexcept override;
    void UpdateHitTestResult(HitTestResult &result, const LayoutPoint &point) const noexcept override;

    RenderInline(Type type, dom::Element &element, RenderStyle &&style) noexcept;
    RenderInline(Type type, dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderInline() noexcept override;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override;

    void ImageChanged(WrappedImagePtr, const IntRect * = nullptr) noexcept override;

    KRYS_NODISCARD LayoutUnit MarginLeft() const noexcept final;
    KRYS_NODISCARD LayoutUnit MarginRight() const noexcept final;
    KRYS_NODISCARD LayoutUnit MarginTop() const noexcept final;
    KRYS_NODISCARD LayoutUnit MarginBottom() const noexcept final;
    KRYS_NODISCARD LayoutUnit MarginBefore(WritingMode writingMode) const noexcept final;
    KRYS_NODISCARD LayoutUnit MarginAfter(WritingMode writingMode) const noexcept final;
    KRYS_NODISCARD LayoutUnit MarginStart(WritingMode writingMode) const noexcept final;
    KRYS_NODISCARD LayoutUnit MarginEnd(WritingMode writingMode) const noexcept final;
    using RenderBoxModelObject::MarginAfter;
    using RenderBoxModelObject::MarginBefore;
    using RenderBoxModelObject::MarginEnd;
    using RenderBoxModelObject::MarginStart;

    KRYS_NODISCARD LayoutRect BorderBoundingBox() const noexcept final
    {
      return LayoutRect(LayoutPoint(), LinesBoundingBox().Size());
    }

    KRYS_NODISCARD LayoutUnit OffsetWidth() const noexcept final
    {
      return LayoutUnit(LinesBoundingBox().Width());
    }
    KRYS_NODISCARD LayoutUnit OffsetHeight() const noexcept final
    {
      return LayoutUnit(LinesBoundingBox().Height());
    }

    KRYS_NODISCARD IntRect LinesBoundingBox() const noexcept;
    /// @brief The bounding box of the inline's line boxes with their ink overflow.
    KRYS_NODISCARD LayoutRect LinesVisualOverflowBoundingBox() const noexcept;
    /// @brief Adds the rects of the inline's line boxes, moved by @p additionalOffset, skipping empty ones.
    void CollectLineBoxRects(List<LayoutRect> &rects, const LayoutPoint &additionalOffset) const noexcept;

    /// @brief The offset an absolutely positioned descendant gets from this in-flow positioned inline.
    KRYS_NODISCARD LayoutSize OffsetForInFlowPositionedInline(const RenderBox *child) const noexcept;

    void MapLocalToContainer(const RenderLayerModelObject *ancestorContainer, TransformState &transformState,
                             EnumSet<MapCoordinatesMode> mode, bool *wasFixed) const noexcept override;
    KRYS_NODISCARD LayoutSize
      OffsetFromContainer(const RenderElement &container, const LayoutPoint &point,
                          bool *offsetDependsOnPoint = nullptr) const noexcept override;
    void BoundingRects(List<LayoutRect> &rects, const LayoutPoint &accumulatedOffset) const noexcept override;
    void AbsoluteQuads(List<FloatQuad> &quads, bool *wasFixed = nullptr) const noexcept override;

    KRYS_NODISCARD bool RequiresLayer() const noexcept override;

#pragma region Repaint

    KRYS_NODISCARD LayoutRect ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                                  VisibleRectContext context) const noexcept override;
    KRYS_NODISCARD RepaintRects RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                                              RepaintOutlineBounds) const noexcept override;
    KRYS_NODISCARD LayoutRect RectWithOutlineForRepaint(const RenderLayerModelObject *repaintContainer,
                                                        LayoutUnit outlineWidth) const noexcept final;
    KRYS_NODISCARD Maybe<RepaintRects>
      ComputeVisibleRectsInContainer(const RepaintRects &rects, const RenderLayerModelObject *container,
                                     VisibleRectContext context) const noexcept final;

  private:
    KRYS_NODISCARD RepaintRects ComputeVisibleRectsUsingPaintOffset(const RepaintRects &rects) const noexcept;

  public:
#pragma endregion

    KRYS_NODISCARD LayoutUnit InnerPaddingBoxWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit InnerPaddingBoxHeight() const noexcept;

  protected:
    void StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept override;
    void UpdateFromStyle() noexcept override;
  };

  /// @brief Whether @p renderer holds nothing but collapsible whitespace (and floating or out-of-flow boxes).
  KRYS_NODISCARD bool IsEmptyInline(const RenderInline &renderer) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderInline)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderInline();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
