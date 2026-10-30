#pragma once

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderLineBoxList.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class LegacyInlineFlowBox;

  /// @brief An inline box.
  class RenderInline : public RenderBoxModelObject
  {
    KRYS_NON_COPY_MOVABLE(RenderInline);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderInline);

  private:
    RenderLineBoxList _legacyLineBoxes;

  public:
    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;
    bool NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                     const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                     HitTestAction hitTestAction) noexcept override;
    void UpdateHitTestResult(HitTestResult &result, const LayoutPoint &point) const noexcept override;
    KRYS_NODISCARD dom::PositionWithAffinity PositionForPoint(const LayoutPoint &point,
                                                              HitTestSource source) noexcept final;

    RenderInline(Type type, dom::Element &element, RenderStyle &&style) noexcept;
    RenderInline(Type type, dom::Document &document, RenderStyle &&style) noexcept;
    ~RenderInline() noexcept override;

    KRYS_NODISCARD bool CanHaveChildren() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override;

    /// @brief Whether removing this inline (leaving its children in its parent) could change the line layout:
    /// its vertical alignment, emphasis marks, font metrics or line height differ from its parent's, or it
    /// holds only a line break.
    KRYS_NODISCARD bool MayAffectLayout() const noexcept;

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
    /// @brief The top left of the inline's first inline box.
    KRYS_NODISCARD LayoutPoint FirstInlineBoxTopLeft() const noexcept;
    KRYS_NODISCARD LayoutUnit OffsetLeft() const noexcept final;
    KRYS_NODISCARD LayoutUnit OffsetTop() const noexcept final;
    KRYS_NODISCARD LayoutRect FrameRectForStickyPositioning() const noexcept final
    {
      return LayoutRect(LinesBoundingBox());
    }
    /// @brief The bounding box of the inline's line boxes with their ink overflow.
    KRYS_NODISCARD LayoutRect LinesVisualOverflowBoundingBox() const noexcept;
    /// @brief Adds the rects of the inline's line boxes, moved by @p additionalOffset, skipping empty ones.
    void CollectLineBoxRects(List<LayoutRect> &rects, const LayoutPoint &additionalOffset) const noexcept;

    /// @brief The offset an absolutely positioned descendant gets from this in-flow positioned inline.
    KRYS_NODISCARD LayoutSize OffsetForInFlowPositionedInline(const RenderBox *child) const noexcept;

    void MapLocalToContainer(const RenderLayerModelObject *ancestorContainer, TransformState &transformState,
                             EnumSet<MapCoordinatesMode> mode, bool *wasFixed) const noexcept override;
    const RenderElement *PushMappingToContainer(const RenderLayerModelObject *ancestorToStopAt,
                                                RenderGeometryMap &geometryMap) const noexcept override;
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

    LegacyInlineFlowBox *CreateAndAppendInlineFlowBox() noexcept;

    KRYS_NODISCARD RenderLineBoxList &LegacyLineBoxes() noexcept
    {
      return _legacyLineBoxes;
    }
    KRYS_NODISCARD const RenderLineBoxList &LegacyLineBoxes() const noexcept
    {
      return _legacyLineBoxes;
    }
    void DeleteLegacyLineBoxes() noexcept;
    KRYS_NODISCARD LegacyInlineFlowBox *FirstLegacyInlineBox() const noexcept
    {
      return _legacyLineBoxes.FirstLegacyLineBox();
    }
    KRYS_NODISCARD LegacyInlineFlowBox *LastLegacyInlineBox() const noexcept
    {
      return _legacyLineBoxes.LastLegacyLineBox();
    }

  protected:
    void WillBeDestroyed() noexcept override;
    void StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept override;
    void UpdateFromStyle() noexcept override;

    /// @brief Subclassed by RenderSVGInline.
    KRYS_NODISCARD virtual UniquePtr<LegacyInlineFlowBox> CreateInlineFlowBox() noexcept;

  private:
    void DirtyLineFromChangedChild() noexcept final
    {
      _legacyLineBoxes.DirtyLineFromChangedChild(*this);
    }
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
