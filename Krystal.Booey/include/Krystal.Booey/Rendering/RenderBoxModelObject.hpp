#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Core/RectEdges.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Rendering/BorderShape.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Style/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Box/Margin.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MaximumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/MinimumSize.hpp"
#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo
{
  class GraphicsContext;
  class Image;
  class RenderBlock;
  class RenderBox;
  class RenderTextFragment;
  class StickyPositionViewportConstraints;
  class StyleImage;

  enum class LineDirection : bool
  {
    Horizontal,
    Vertical
  };

  enum class ContentChangeType : uint8
  {
    Image,
    HDRImage,
    MaskImage,
    BackgroundImage,
    Canvas,
    CanvasPixels,
    Video,
    FullScreen,
    Model
  };

  enum class BleedAvoidance : uint8
  {
    None,
    ShrinkBackground,
    UseTransparencyLayer,
    BackgroundOverBorder
  };

  /// @brief A renderer that follows the CSS box model: a box or an inline box.
  // FIXME: Continuations come with inline painting.
  class RenderBoxModelObject : public RenderLayerModelObject
  {
    KRYS_NON_COPY_MOVABLE(RenderBoxModelObject);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderBoxModelObject);

  public:
    ~RenderBoxModelObject() noexcept override;

    KRYS_NODISCARD bool HasVisibleBoxDecorationStyle() const noexcept;

    KRYS_NODISCARD bool RequiresLayer() const noexcept override;

    void ApplyTransform(TransformationMatrix &transform, const RenderStyle &style,
                        const FloatRect &boundingBox,
                        EnumSet<style::TransformResolverOption> options) const noexcept override;
    using RenderLayerModelObject::ApplyTransform;

    KRYS_NODISCARD LayoutSize RelativePositionOffset() const noexcept;

    void ContentChanged(ContentChangeType changeType, const Maybe<FloatRect> &dirtyRect = null) noexcept;
    KRYS_NODISCARD bool HasAcceleratedCompositing() const noexcept;
    KRYS_NODISCARD bool HasRunningAcceleratedAnimations() const noexcept;
    /// @brief The rect a sticky box stays in: its scroller's scrolled clip rect, or the view's.
    KRYS_NODISCARD FloatRect ConstrainingRectForStickyPosition() const noexcept;
    /// @brief The box a sticky box scrolls with (its nearest overflow clip, or the view) and that box's
    /// layer, if it's a clip.
    KRYS_NODISCARD Pair<const RenderBox &, const RenderLayer *>
      EnclosingClippingBoxForStickyPosition() const noexcept;
    void ComputeStickyPositionConstraints(StickyPositionViewportConstraints &constraints,
                                          const FloatRect &constrainingRect) const noexcept;
    /// @brief How far a sticky box moves from where it's laid out to stay in view.
    KRYS_NODISCARD LayoutSize StickyPositionOffset() const noexcept;
    KRYS_NODISCARD LayoutSize OffsetForInFlowPosition() const noexcept;

    void MapAbsoluteToLocalPoint(EnumSet<MapCoordinatesMode> mode,
                                 TransformState &transformState) const noexcept override;

    KRYS_NODISCARD virtual LayoutUnit OffsetLeft() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit OffsetTop() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit OffsetWidth() const noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit OffsetHeight() const noexcept = 0;

    KRYS_NODISCARD virtual LayoutRect BorderBoundingBox() const noexcept = 0;
    /// @brief The rect sticky positioning moves, relative to the container.
    KRYS_NODISCARD virtual LayoutRect FrameRectForStickyPositioning() const noexcept = 0;

    // Used to resolve the CSS padding values, which can be percentages of the containing block's width.
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingTop() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingBottom() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingLeft() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingRight() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingBefore() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingAfter() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingStart() const noexcept;
    KRYS_NODISCARD LayoutUnit ComputedCSSPaddingEnd() const noexcept;

    KRYS_NODISCARD virtual RectEdges<LayoutUnit> Padding() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit PaddingTop() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit PaddingBottom() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit PaddingLeft() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit PaddingRight() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit PaddingBefore() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit PaddingAfter() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit PaddingStart() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit PaddingEnd() const noexcept;

    KRYS_NODISCARD virtual RectEdges<LayoutUnit> BorderWidths() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit BorderTop() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit BorderBottom() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit BorderLeft() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit BorderRight() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit HorizontalBorderExtent() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit VerticalBorderExtent() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit BorderBefore() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit BorderAfter() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit BorderStart() const noexcept;
    KRYS_NODISCARD virtual LayoutUnit BorderEnd() const noexcept;

    KRYS_NODISCARD LayoutUnit BorderAndPaddingStart() const noexcept;
    KRYS_NODISCARD LayoutUnit BorderAndPaddingEnd() const noexcept;
    KRYS_NODISCARD LayoutUnit BorderAndPaddingBefore() const noexcept;
    KRYS_NODISCARD LayoutUnit BorderAndPaddingAfter() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginAndBorderAndPaddingStart() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginAndBorderAndPaddingEnd() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginAndBorderAndPaddingBefore() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginAndBorderAndPaddingAfter() const noexcept;
    KRYS_NODISCARD LayoutUnit VerticalBorderAndPaddingExtent() const noexcept;
    KRYS_NODISCARD LayoutUnit HorizontalBorderAndPaddingExtent() const noexcept;
    KRYS_NODISCARD LayoutUnit BorderAndPaddingLogicalHeight() const noexcept;
    KRYS_NODISCARD LayoutUnit BorderAndPaddingLogicalWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit BorderAndPaddingLogicalLeft() const noexcept;
    KRYS_NODISCARD LayoutUnit BorderAndPaddingLogicalRight() const noexcept;
    KRYS_NODISCARD LayoutUnit BorderLogicalLeft() const noexcept;
    KRYS_NODISCARD LayoutUnit BorderLogicalRight() const noexcept;
    KRYS_NODISCARD LayoutUnit BorderLogicalWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit BorderLogicalHeight() const noexcept;
    KRYS_NODISCARD LayoutUnit PaddingLogicalLeft() const noexcept;
    KRYS_NODISCARD LayoutUnit PaddingLogicalRight() const noexcept;
    KRYS_NODISCARD LayoutUnit PaddingLogicalWidth() const noexcept;
    KRYS_NODISCARD LayoutUnit PaddingLogicalHeight() const noexcept;

    KRYS_NODISCARD virtual LayoutUnit MarginTop() const noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit MarginBottom() const noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit MarginLeft() const noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit MarginRight() const noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit MarginBefore(WritingMode writingMode) const noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit MarginAfter(WritingMode writingMode) const noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit MarginStart(WritingMode writingMode) const noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit MarginEnd(WritingMode writingMode) const noexcept = 0;
    KRYS_NODISCARD LayoutUnit MarginBefore() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginAfter() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginStart() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginEnd() const noexcept;
    KRYS_NODISCARD LayoutUnit VerticalMarginExtent() const noexcept;
    KRYS_NODISCARD LayoutUnit HorizontalMarginExtent() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginLogicalHeight() const noexcept;
    KRYS_NODISCARD LayoutUnit MarginLogicalWidth() const noexcept;

    KRYS_NODISCARD bool HasInlineDirectionBordersPaddingOrMargin() const noexcept;
    KRYS_NODISCARD bool HasInlineDirectionBordersOrPadding() const noexcept;

    KRYS_NODISCARD virtual LayoutUnit ContainingBlockLogicalWidthForContent() const noexcept;

    KRYS_NODISCARD RenderBlock *
      ContainingBlockForAutoHeightDetection(const style::PreferredSize &logicalHeight) const noexcept;
    KRYS_NODISCARD RenderBlock *
      ContainingBlockForAutoHeightDetection(const style::MinimumSize &logicalHeight) const noexcept;
    KRYS_NODISCARD RenderBlock *
      ContainingBlockForAutoHeightDetection(const style::MaximumSize &logicalHeight) const noexcept;

    /// @brief For a `::first-letter` renderer, the text after the first letter.
    KRYS_NODISCARD RenderTextFragment *FirstLetterRemainingText() const noexcept;
    void SetFirstLetterRemainingText(RenderTextFragment &remainingText) noexcept;
    void ClearFirstLetterRemainingText() noexcept;

  public:
    enum class ScaleByUsedZoom : bool
    {
      No,
      Yes
    };

    /// @brief The size @p image is drawn at in a positioning area of @p positioningAreaSize, from its natural
    /// size and ratio where it has them.
    KRYS_NODISCARD LayoutSize CalculateImageIntrinsicDimensions(StyleImage *image,
                                                                const LayoutSize &positioningAreaSize,
                                                                ScaleByUsedZoom) const noexcept;

    /// @brief Whether the root's fixed background is painted in its layer's coordinates: never, as there's no
    /// compositing.
    KRYS_NODISCARD bool FixedBackgroundPaintsInLocalCoordinates() const noexcept;

    /// @brief How @p image is interpolated when drawn at @p size.
    /// FIXME: The image quality controller (low quality while an image is resized live) comes with bitmap
    /// images.
    KRYS_NODISCARD InterpolationQuality ChooseInterpolationQuality(GraphicsContext &, Image &, const void *,
                                                                   const LayoutSize &) const noexcept
    {
      return InterpolationQuality::Default;
    }

    /// @brief Whether every side of the border is opaque and at least two device pixels wide (at
    /// @p contextScale), so the background's edge needn't show beneath it.
    KRYS_NODISCARD bool BorderObscuresBackgroundEdge(const FloatSize &contextScale) const noexcept;
    /// @brief Whether every side of the border is opaque and solid, so the background needn't show beneath
    /// it.
    KRYS_NODISCARD bool BorderObscuresBackground() const noexcept;

    /// @brief The shape of the content box, inset from the border box by the borders and the padding.
    KRYS_NODISCARD BorderShape BorderShapeForContentClipping(const LayoutRect &borderBoxRect,
                                                             RectEdges<bool> closedEdges = {
                                                               true}) const noexcept;

  protected:
    void RemoveOutOfFlowBoxesIfNeededOnStyleChange(RenderBlock &delegateBlock, const RenderStyle &oldStyle,
                                                   const RenderStyle &newStyle) noexcept;
    RenderBoxModelObject(Type type, dom::Element &element, RenderStyle &&style,
                         EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept;
    RenderBoxModelObject(Type type, dom::Document &document, RenderStyle &&style,
                         EnumSet<TypeFlag> baseTypeFlags, TypeSpecificFlags typeSpecificFlags) noexcept;

    void WillBeDestroyed() noexcept override;
    void UpdateFromStyle() noexcept override;

    KRYS_NODISCARD LayoutPoint
      AdjustedPositionRelativeToOffsetParent(const LayoutPoint &startPoint) const noexcept;

    KRYS_NODISCARD LayoutUnit
      ResolveLengthPercentageUsingContainerLogicalWidth(const auto &value) const noexcept
    {
      LayoutUnit containerWidth;
      if (value.IsPercentOrCalculated())
      {
        containerWidth = ContainingBlockLogicalWidthForContent();
      }
      return style::EvaluateMinimum<LayoutUnit>(value, containerWidth, style::ZoomNeeded {});
    }
    KRYS_NODISCARD LayoutUnit ResolveLengthPercentageUsingContainerLogicalWidth(
      const auto &value, const style::ZoomFactor &zoomFactor) const noexcept
    {
      LayoutUnit containerWidth;
      if (value.IsPercentOrCalculated())
      {
        containerWidth = ContainingBlockLogicalWidthForContent();
      }
      return style::EvaluateMinimum<LayoutUnit>(value, containerWidth, zoomFactor);
    }

  private:
    KRYS_NODISCARD RenderBlock *
      ContainingBlockForAutoHeightDetectionGeneric(const auto &logicalHeight) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderBoxModelObject)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderBoxModelObject();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
