#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Scrolling/ScrollingConstraints.hpp"
#include "Krystal.Booey/Rendering/BorderEdge.hpp"
#include "Krystal.Booey/Rendering/ImageQualityController.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderTableRow.hpp"
#include "Krystal.Booey/Rendering/RenderTextFragment.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Booey/Style/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  namespace
  {
    // Entries are removed when their first-letter renderer is destroyed.
    using FirstLetterRemainingTextMap = HashMap<const RenderBoxModelObject *, WeakPtr<RenderTextFragment>>;

    KRYS_NODISCARD FirstLetterRemainingTextMap &GetFirstLetterRemainingTextMap() noexcept
    {
      static FirstLetterRemainingTextMap map;
      return map;
    }
  }

  RenderBoxModelObject::RenderBoxModelObject(Type type, dom::Element &element, RenderStyle &&style,
                                             EnumSet<TypeFlag> baseTypeFlags,
                                             TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderLayerModelObject(type, element, krys::move(style), baseTypeFlags | TypeFlag::IsBoxModelObject,
                               typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBoxModelObject());
  }

  RenderBoxModelObject::RenderBoxModelObject(Type type, dom::Document &document, RenderStyle &&style,
                                             EnumSet<TypeFlag> baseTypeFlags,
                                             TypeSpecificFlags typeSpecificFlags) noexcept
      : RenderLayerModelObject(type, document, krys::move(style), baseTypeFlags | TypeFlag::IsBoxModelObject,
                               typeSpecificFlags)
  {
    krys_debug_assert(IsRenderBoxModelObject());
  }

  RenderBoxModelObject::~RenderBoxModelObject() noexcept = default;

  void RenderBoxModelObject::SetSelectionState(HighlightState state) noexcept
  {
    if (state == HighlightState::Inside && SelectionState() != HighlightState::None)
    {
      return;
    }

    if ((state == HighlightState::Start && SelectionState() == HighlightState::End)
        || (state == HighlightState::End && SelectionState() == HighlightState::Start))
    {
      RenderLayerModelObject::SetSelectionState(HighlightState::Both);
    }
    else
    {
      RenderLayerModelObject::SetSelectionState(state);
    }

    // FIXME: We should consider whether it is OK propagating to ancestor RenderInlines.
    // The containing block can be null in case of an orphaned tree.
    auto *containingBlock = ContainingBlock();
    if (containingBlock && !containingBlock->IsRenderView())
    {
      containingBlock->SetSelectionState(state);
    }
  }

  void RenderBoxModelObject::WillBeDestroyed() noexcept
  {
    // FIXME: The image quality controller, once there's painting.
    if (IsFirstLetter())
    {
      GetFirstLetterRemainingTextMap().erase(this);
    }

    View().UnregisterAnchor(*this);

    RenderLayerModelObject::WillBeDestroyed();
  }

  void RenderBoxModelObject::StyleWillChange(style::Difference diff, const RenderStyle &newStyle) noexcept
  {
    const auto *oldStyle = HasInitializedStyle() ? &Style() : nullptr;

    if (style::AnchorPositionEvaluator::IsAnchor(newStyle))
    {
      View().RegisterAnchor(*this);
    }
    else if (oldStyle && style::AnchorPositionEvaluator::IsAnchor(*oldStyle))
    {
      View().UnregisterAnchor(*this);
    }

    RenderLayerModelObject::StyleWillChange(diff, newStyle);
  }

  RenderTextFragment *RenderBoxModelObject::FirstLetterRemainingText() const noexcept
  {
    if (!IsFirstLetter())
    {
      return nullptr;
    }
    auto &map = GetFirstLetterRemainingTextMap();
    auto it = map.find(this);
    return it != map.end() ? it->second.get() : nullptr;
  }

  void RenderBoxModelObject::SetFirstLetterRemainingText(RenderTextFragment &remainingText) noexcept
  {
    krys_debug_assert(IsFirstLetter());
    GetFirstLetterRemainingTextMap()[this] = CreateWeakPtr(&remainingText);
  }

  void RenderBoxModelObject::ClearFirstLetterRemainingText() noexcept
  {
    krys_debug_assert(IsFirstLetter());
    GetFirstLetterRemainingTextMap().erase(this);
  }

  bool RenderBoxModelObject::HasVisibleBoxDecorationStyle() const noexcept
  {
    return Style().HasBackground() || Style().HasVisibleBorderDecoration() || Style().HasUsedAppearance()
           || Style().HasBoxShadow();
  }

  void RenderBoxModelObject::UpdateFromStyle() noexcept
  {
    RenderLayerModelObject::UpdateFromStyle();

    // Set the appropriate bits for a box model object.  Since all bits are cleared in styleWillChange,
    // we only check for bits that could possibly be set to true.
    const auto &styleToUse = Style();
    SetHasVisibleBoxDecorations(HasVisibleBoxDecorationStyle());
    SetInline(styleToUse.IsDisplayInlineType());
    SetPositionState(styleToUse.GetPosition());
    SetHorizontalWritingMode(styleToUse.GetWritingMode().IsHorizontal());
    SetPaintContainmentApplies(ShouldApplyPaintContainment());
    if (Style().GetWritingMode().IsBlockFlipped())
    {
      View().FrameView().SetHasFlippedBlockRenderers(true);
    }
  }

#pragma region Geometry

  LayoutUnit RenderBoxModelObject::ComputedCSSPaddingTop() const noexcept
  {
    return ResolveLengthPercentageUsingContainerLogicalWidth(Style().PaddingTop(),
                                                             Style().UsedZoomForLength());
  }

  LayoutUnit RenderBoxModelObject::ComputedCSSPaddingBottom() const noexcept
  {
    return ResolveLengthPercentageUsingContainerLogicalWidth(Style().PaddingBottom(),
                                                             Style().UsedZoomForLength());
  }

  LayoutUnit RenderBoxModelObject::ComputedCSSPaddingLeft() const noexcept
  {
    return ResolveLengthPercentageUsingContainerLogicalWidth(Style().PaddingLeft(),
                                                             Style().UsedZoomForLength());
  }

  LayoutUnit RenderBoxModelObject::ComputedCSSPaddingRight() const noexcept
  {
    return ResolveLengthPercentageUsingContainerLogicalWidth(Style().PaddingRight(),
                                                             Style().UsedZoomForLength());
  }

  LayoutUnit RenderBoxModelObject::ComputedCSSPaddingBefore() const noexcept
  {
    return ResolveLengthPercentageUsingContainerLogicalWidth(Style().PaddingBefore(),
                                                             Style().UsedZoomForLength());
  }

  LayoutUnit RenderBoxModelObject::ComputedCSSPaddingAfter() const noexcept
  {
    return ResolveLengthPercentageUsingContainerLogicalWidth(Style().PaddingAfter(),
                                                             Style().UsedZoomForLength());
  }

  LayoutUnit RenderBoxModelObject::ComputedCSSPaddingStart() const noexcept
  {
    return ResolveLengthPercentageUsingContainerLogicalWidth(Style().PaddingStart(),
                                                             Style().UsedZoomForLength());
  }

  LayoutUnit RenderBoxModelObject::ComputedCSSPaddingEnd() const noexcept
  {
    return ResolveLengthPercentageUsingContainerLogicalWidth(Style().PaddingEnd(),
                                                             Style().UsedZoomForLength());
  }

  RectEdges<LayoutUnit> RenderBoxModelObject::Padding() const noexcept
  {
    return {ComputedCSSPaddingTop(), ComputedCSSPaddingRight(), ComputedCSSPaddingBottom(),
            ComputedCSSPaddingLeft()};
  }

  LayoutUnit RenderBoxModelObject::PaddingTop() const noexcept
  {
    return ComputedCSSPaddingTop();
  }

  LayoutUnit RenderBoxModelObject::PaddingBottom() const noexcept
  {
    return ComputedCSSPaddingBottom();
  }

  LayoutUnit RenderBoxModelObject::PaddingLeft() const noexcept
  {
    return ComputedCSSPaddingLeft();
  }

  LayoutUnit RenderBoxModelObject::PaddingRight() const noexcept
  {
    return ComputedCSSPaddingRight();
  }

  LayoutUnit RenderBoxModelObject::PaddingBefore() const noexcept
  {
    return ComputedCSSPaddingBefore();
  }

  LayoutUnit RenderBoxModelObject::PaddingAfter() const noexcept
  {
    return ComputedCSSPaddingAfter();
  }

  LayoutUnit RenderBoxModelObject::PaddingStart() const noexcept
  {
    return ComputedCSSPaddingStart();
  }

  LayoutUnit RenderBoxModelObject::PaddingEnd() const noexcept
  {
    return ComputedCSSPaddingEnd();
  }

  RectEdges<LayoutUnit> RenderBoxModelObject::BorderWidths() const noexcept
  {
    return {BorderTop(), BorderRight(), BorderBottom(), BorderLeft()};
  }

  LayoutUnit RenderBoxModelObject::BorderTop() const noexcept
  {
    return style::Evaluate<LayoutUnit>(Style().BorderTopWidth(), style::ZoomNeeded {});
  }

  LayoutUnit RenderBoxModelObject::BorderBottom() const noexcept
  {
    return style::Evaluate<LayoutUnit>(Style().BorderBottomWidth(), style::ZoomNeeded {});
  }

  LayoutUnit RenderBoxModelObject::BorderLeft() const noexcept
  {
    return style::Evaluate<LayoutUnit>(Style().BorderLeftWidth(), style::ZoomNeeded {});
  }

  LayoutUnit RenderBoxModelObject::BorderRight() const noexcept
  {
    return style::Evaluate<LayoutUnit>(Style().BorderRightWidth(), style::ZoomNeeded {});
  }

  LayoutUnit RenderBoxModelObject::HorizontalBorderExtent() const noexcept
  {
    return BorderLeft() + BorderRight();
  }

  LayoutUnit RenderBoxModelObject::VerticalBorderExtent() const noexcept
  {
    return BorderTop() + BorderBottom();
  }

  LayoutUnit RenderBoxModelObject::BorderBefore() const noexcept
  {
    return style::Evaluate<LayoutUnit>(Style().BorderWidthBefore(), style::ZoomNeeded {});
  }

  LayoutUnit RenderBoxModelObject::BorderAfter() const noexcept
  {
    return style::Evaluate<LayoutUnit>(Style().BorderWidthAfter(), style::ZoomNeeded {});
  }

  LayoutUnit RenderBoxModelObject::BorderStart() const noexcept
  {
    return style::Evaluate<LayoutUnit>(Style().BorderWidthStart(), style::ZoomNeeded {});
  }

  LayoutUnit RenderBoxModelObject::BorderEnd() const noexcept
  {
    return style::Evaluate<LayoutUnit>(Style().BorderWidthEnd(), style::ZoomNeeded {});
  }

  LayoutUnit RenderBoxModelObject::BorderAndPaddingStart() const noexcept
  {
    return BorderStart() + PaddingStart();
  }

  LayoutUnit RenderBoxModelObject::BorderAndPaddingEnd() const noexcept
  {
    return BorderEnd() + PaddingEnd();
  }

  LayoutUnit RenderBoxModelObject::BorderAndPaddingBefore() const noexcept
  {
    return BorderBefore() + PaddingBefore();
  }

  LayoutUnit RenderBoxModelObject::BorderAndPaddingAfter() const noexcept
  {
    return BorderAfter() + PaddingAfter();
  }

  LayoutUnit RenderBoxModelObject::MarginAndBorderAndPaddingStart() const noexcept
  {
    return MarginStart() + BorderStart() + PaddingStart();
  }

  LayoutUnit RenderBoxModelObject::MarginAndBorderAndPaddingEnd() const noexcept
  {
    return MarginEnd() + BorderEnd() + PaddingEnd();
  }

  LayoutUnit RenderBoxModelObject::MarginAndBorderAndPaddingBefore() const noexcept
  {
    return MarginBefore() + BorderBefore() + PaddingBefore();
  }

  LayoutUnit RenderBoxModelObject::MarginAndBorderAndPaddingAfter() const noexcept
  {
    return MarginAfter() + BorderAfter() + PaddingAfter();
  }

  LayoutUnit RenderBoxModelObject::VerticalBorderAndPaddingExtent() const noexcept
  {
    return BorderTop() + BorderBottom() + PaddingTop() + PaddingBottom();
  }

  LayoutUnit RenderBoxModelObject::HorizontalBorderAndPaddingExtent() const noexcept
  {
    return BorderLeft() + BorderRight() + PaddingLeft() + PaddingRight();
  }

  LayoutUnit RenderBoxModelObject::BorderAndPaddingLogicalHeight() const noexcept
  {
    return BorderAndPaddingBefore() + BorderAndPaddingAfter();
  }

  LayoutUnit RenderBoxModelObject::BorderAndPaddingLogicalWidth() const noexcept
  {
    return BorderStart() + BorderEnd() + PaddingStart() + PaddingEnd();
  }

  LayoutUnit RenderBoxModelObject::BorderAndPaddingLogicalLeft() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? BorderLeft() + PaddingLeft()
                                                   : BorderTop() + PaddingTop();
  }

  LayoutUnit RenderBoxModelObject::BorderAndPaddingLogicalRight() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? BorderRight() + PaddingRight()
                                                   : BorderBottom() + PaddingBottom();
  }

  LayoutUnit RenderBoxModelObject::BorderLogicalLeft() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? BorderLeft() : BorderTop();
  }

  LayoutUnit RenderBoxModelObject::BorderLogicalRight() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? BorderRight() : BorderBottom();
  }

  LayoutUnit RenderBoxModelObject::BorderLogicalWidth() const noexcept
  {
    return BorderStart() + BorderEnd();
  }

  LayoutUnit RenderBoxModelObject::BorderLogicalHeight() const noexcept
  {
    return BorderBefore() + BorderAfter();
  }

  LayoutUnit RenderBoxModelObject::PaddingLogicalLeft() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? PaddingLeft() : PaddingTop();
  }

  LayoutUnit RenderBoxModelObject::PaddingLogicalRight() const noexcept
  {
    return Style().GetWritingMode().IsHorizontal() ? PaddingRight() : PaddingBottom();
  }

  LayoutUnit RenderBoxModelObject::PaddingLogicalWidth() const noexcept
  {
    return PaddingStart() + PaddingEnd();
  }

  LayoutUnit RenderBoxModelObject::PaddingLogicalHeight() const noexcept
  {
    return PaddingBefore() + PaddingAfter();
  }

  LayoutUnit RenderBoxModelObject::MarginBefore() const noexcept
  {
    return MarginBefore(Style().GetWritingMode());
  }

  LayoutUnit RenderBoxModelObject::MarginAfter() const noexcept
  {
    return MarginAfter(Style().GetWritingMode());
  }

  LayoutUnit RenderBoxModelObject::MarginStart() const noexcept
  {
    return MarginStart(Style().GetWritingMode());
  }

  LayoutUnit RenderBoxModelObject::MarginEnd() const noexcept
  {
    return MarginEnd(Style().GetWritingMode());
  }

  LayoutUnit RenderBoxModelObject::VerticalMarginExtent() const noexcept
  {
    return MarginTop() + MarginBottom();
  }

  LayoutUnit RenderBoxModelObject::HorizontalMarginExtent() const noexcept
  {
    return MarginLeft() + MarginRight();
  }

  LayoutUnit RenderBoxModelObject::MarginLogicalHeight() const noexcept
  {
    return MarginBefore() + MarginAfter();
  }

  LayoutUnit RenderBoxModelObject::MarginLogicalWidth() const noexcept
  {
    return MarginStart() + MarginEnd();
  }

  bool RenderBoxModelObject::HasInlineDirectionBordersOrPadding() const noexcept
  {
    return BorderStart() || BorderEnd() || PaddingStart() || PaddingEnd();
  }

  bool RenderBoxModelObject::HasInlineDirectionBordersPaddingOrMargin() const noexcept
  {
    return HasInlineDirectionBordersOrPadding() || MarginStart() || MarginEnd();
  }

  LayoutUnit RenderBoxModelObject::ContainingBlockLogicalWidthForContent() const noexcept
  {
    if (auto *containingBlock = ContainingBlock())
    {
      return containingBlock->ContentBoxLogicalWidth();
    }
    return {};
  }

  static LayoutSize AccumulateInFlowPositionOffsets(const RenderBoxModelObject &) noexcept
  {
    // FIXME: The in-flow offsets of the inline ancestors of an anonymous block's inline continuation, once
    // there are continuations.
    return {};
  }

  static bool IsOutOfFlowPositionedWithImplicitHeight(const RenderBoxModelObject &child) noexcept
  {
    return child.IsOutOfFlowPositioned() && !child.Style().InsetBefore().IsAuto()
           && !child.Style().InsetAfter().IsAuto();
  }

  RenderBlock *RenderBoxModelObject::ContainingBlockForAutoHeightDetectionGeneric(
    const auto &logicalHeight) const noexcept
  {
    // For percentage heights: The percentage is calculated with respect to the
    // height of the generated box's containing block. If the height of the
    // containing block is not specified explicitly (i.e., it depends on content
    // height), and this element is not absolutely positioned, the used height is
    // calculated as if 'auto' was specified.
    if (!logicalHeight.IsPercentOrCalculated() || IsOutOfFlowPositioned())
    {
      return nullptr;
    }

    // Anonymous block boxes are ignored when resolving percentage values that
    // would refer to it: the closest non-anonymous ancestor box is used instead.
    auto *cb = ContainingBlock();
    while (cb && cb->IsAnonymousForPercentageResolution() && !Is<RenderView>(cb))
    {
      cb = cb->ContainingBlock();
    }
    if (!cb)
    {
      return nullptr;
    }

    // Matching RenderBox::percentageLogicalHeightIsResolvable() by
    // ignoring table cell's attribute value, where it says that table cells
    // violate what the CSS spec says to do with heights. Basically we don't care
    // if the cell specified a height or not.
    if (cb->IsRenderTableCell())
    {
      return nullptr;
    }

    // Match RenderBox::availableLogicalHeightUsing by special casing the layout
    // view. The available height is taken from the frame.
    if (cb->IsRenderView())
    {
      return nullptr;
    }

    if (IsOutOfFlowPositionedWithImplicitHeight(*cb))
    {
      return nullptr;
    }

    return cb;
  }

  RenderBlock *RenderBoxModelObject::ContainingBlockForAutoHeightDetection(
    const style::PreferredSize &logicalHeight) const noexcept
  {
    return ContainingBlockForAutoHeightDetectionGeneric(logicalHeight);
  }

  RenderBlock *RenderBoxModelObject::ContainingBlockForAutoHeightDetection(
    const style::MinimumSize &logicalHeight) const noexcept
  {
    return ContainingBlockForAutoHeightDetectionGeneric(logicalHeight);
  }

  RenderBlock *RenderBoxModelObject::ContainingBlockForAutoHeightDetection(
    const style::MaximumSize &logicalHeight) const noexcept
  {
    return ContainingBlockForAutoHeightDetectionGeneric(logicalHeight);
  }

  void RenderBoxModelObject::ContentChanged(ContentChangeType changeType,
                                            const Maybe<FloatRect> &dirtyRect) noexcept
  {
    if (!HasLayer())
    {
      return;
    }

    Layer()->ContentChanged(changeType, dirtyRect);
  }

  bool RenderBoxModelObject::HasRunningAcceleratedAnimations() const noexcept
  {
    if (auto styleable = style::Styleable::FromRenderer(*this))
    {
      return styleable->HasRunningAcceleratedAnimations();
    }
    return false;
  }

  bool RenderBoxModelObject::HasAcceleratedCompositing() const noexcept
  {
    return View().Compositor().HasAcceleratedCompositing();
  }

  LayoutSize RenderBoxModelObject::RelativePositionOffset() const noexcept
  {
    auto *containingBlock = ContainingBlock();

    const auto &style = Style();
    const auto &left = style.Left();
    const auto &right = style.Right();
    const auto &top = style.Top();
    const auto &bottom = style.Bottom();

    auto offset = AccumulateInFlowPositionOffsets(*this);
    auto topFixed = top.TryFixed();
    auto leftFixed = left.TryFixed();
    if (topFixed && leftFixed && bottom.IsAuto() && right.IsAuto()
        && containingBlock->Style().GetWritingMode().IsAnyLeftToRight())
    {
      offset.Expand(LayoutUnit(leftFixed->ResolveZoom(style.UsedZoomForLength())),
                    LayoutUnit(topFixed->ResolveZoom(style.UsedZoomForLength())));
      return offset;
    }

    // Objects that shrink to avoid floats normally use available line width when computing containing block
    // width. However in the case of relative positioning using percentages, we can't do this. The offset
    // should always be resolved using the available width of the containing block. Therefore we don't use
    // containingBlockLogicalWidthForContent() here, but instead explicitly call availableWidth on our
    // containing block.
    if (!left.IsAuto() || !right.IsAuto())
    {
      auto availableWidth = [&]
      {
        auto *renderBox = DynamicDowncast<RenderBox>(*this);
        if (!renderBox || !renderBox->IsGridItem())
        {
          return containingBlock->ContentBoxWidth();
        }
        // For grid items the containing block is the grid area, so offsets should be resolved against that.
        auto containingBlockContentWidth =
          renderBox->GridAreaContentWidth(containingBlock->Style().GetWritingMode());
        if (!containingBlockContentWidth || !*containingBlockContentWidth)
        {
          krys_debug_assert(false);
          return containingBlock->ContentBoxWidth();
        }
        return **containingBlockContentWidth;
      };
      if (!left.IsAuto())
      {
        if (!right.IsAuto() && !containingBlock->Style().GetWritingMode().IsAnyLeftToRight())
        {
          offset.SetWidth(-style::Evaluate<LayoutUnit>(right, !right.IsFixed() ? availableWidth() : 0_lu,
                                                       style.UsedZoomForLength()));
        }
        else
        {
          offset.Expand(style::Evaluate<LayoutUnit>(left, !left.IsFixed() ? availableWidth() : 0_lu,
                                                    style.UsedZoomForLength()),
                        0_lu);
        }
      }
      else if (!right.IsAuto())
      {
        offset.Expand(-style::Evaluate<LayoutUnit>(right, !right.IsFixed() ? availableWidth() : 0_lu,
                                                   style.UsedZoomForLength()),
                      0_lu);
      }
    }

    // If the containing block of a relatively positioned element does not
    // specify a height, a percentage top or bottom offset should be resolved as
    // auto. An exception to this is if the containing block has the WinIE quirk
    // where <html> and <body> assume the size of the viewport. In this case,
    // calculate the percent offset based on this height.
    // See <https://bugs.webkit.org/show_bug.cgi?id=26396>.
    // Another exception is a grid item, as the containing block is the grid area:
    // https://drafts.csswg.org/css-grid/#grid-item-sizing
    if (top.IsAuto() && bottom.IsAuto())
    {
      return offset;
    }

    auto containingBlockHasDefiniteHeight = !containingBlock->HasAutoHeightOrContainingBlockWithAutoHeight()
                                            || containingBlock->StretchesToViewport();
    auto availableHeight = [&]
    {
      auto *renderBox = DynamicDowncast<RenderBox>(*this);
      if (!renderBox || !renderBox->IsGridItem())
      {
        return containingBlock->ContentBoxHeight();
      }
      // For grid items the containing block is the grid area, so offsets should be resolved against that.
      auto containingBlockContentHeight =
        renderBox->GridAreaContentHeight(containingBlock->Style().GetWritingMode());
      if (!containingBlockContentHeight || !*containingBlockContentHeight)
      {
        krys_debug_assert(false);
        return containingBlock->ContentBoxHeight();
      }
      return **containingBlockContentHeight;
    };
    if (!top.IsAuto() && (!top.IsPercentOrCalculated() || containingBlockHasDefiniteHeight))
    {
      // FIXME: The computation of the available height is repeated later for "bottom".
      // We could refactor this and move it to some common code for both ifs, however moving it outside of the
      // ifs is not possible as it'd cause performance regressions.
      offset.Expand(0_lu, style::Evaluate<LayoutUnit>(top, !top.IsFixed() ? availableHeight() : 0_lu,
                                                      style.UsedZoomForLength()));
    }
    else if (!bottom.IsAuto() && (!bottom.IsPercentOrCalculated() || containingBlockHasDefiniteHeight))
    {
      // FIXME: Check comment above for "top", it applies here too.
      offset.Expand(0_lu, -style::Evaluate<LayoutUnit>(bottom, !bottom.IsFixed() ? availableHeight() : 0_lu,
                                                       style.UsedZoomForLength()));
    }
    return offset;
  }

  void RenderBoxModelObject::MapAbsoluteToLocalPoint(EnumSet<MapCoordinatesMode> mode,
                                                     TransformState &transformState) const noexcept
  {
    auto *container = Container();
    if (!container)
    {
      return;
    }

    container->MapAbsoluteToLocalPoint(mode, transformState);

    auto containerOffset = OffsetFromContainer(*container, LayoutPoint());

    PushOntoTransformState(transformState, mode, nullptr, container, containerOffset, false);
  }

  LayoutSize RenderBoxModelObject::OffsetForInFlowPosition() const noexcept
  {
    if (IsRelativelyPositioned())
    {
      return RelativePositionOffset();
    }

    if (IsStickilyPositioned())
    {
      return StickyPositionOffset();
    }

    return {};
  }

  Pair<const RenderBox &, const RenderLayer *>
    RenderBoxModelObject::EnclosingClippingBoxForStickyPosition() const noexcept
  {
    krys_debug_assert(IsStickilyPositioned());
    RenderLayer *clipLayer = HasLayer() ? Layer()->EnclosingOverflowClipLayer(ExcludeSelf) : nullptr;
    const RenderBox &box = clipLayer ? Downcast<RenderBox>(clipLayer->Renderer()) : View();
    return {box, clipLayer};
  }

  void
    RenderBoxModelObject::ComputeStickyPositionConstraints(StickyPositionViewportConstraints &constraints,
                                                           const FloatRect &constrainingRect) const noexcept
  {
    constraints.SetConstrainingRectAtLastLayout(constrainingRect);

    // Do not use anonymous containing blocks to determine sticky constraints. We want the size of the first
    // true containing block, because that is what imposes the limitation on the movement of stickily
    // positioned items.
    RenderBlock *containingBlock = ContainingBlock();
    while (containingBlock && (!Is<RenderBlock>(*containingBlock) || containingBlock->IsAnonymousBlock()))
    {
      containingBlock = containingBlock->ContainingBlock();
    }
    krys_debug_assert(containingBlock);

    auto [enclosingClippingBox, enclosingClippingLayer] = EnclosingClippingBoxForStickyPosition();

    LayoutRect containerContentRect;
    if (!enclosingClippingLayer || (containingBlock != &enclosingClippingBox))
    {
      // In this case either the scrolling element is the view or there is another containing block in the
      // hierarchy between this stickily positioned item and its scrolling ancestor. In both cases, we use the
      // content box rectangle of the containing block, which is what should constrain the movement.
      containerContentRect = containingBlock->ComputedCSSContentBoxRect();
    }
    else
    {
      containerContentRect = containingBlock->LayoutOverflowRect();
      containerContentRect.Contract(LayoutBoxExtent {
        containingBlock->ComputedCSSPaddingTop(), containingBlock->ComputedCSSPaddingRight(),
        containingBlock->ComputedCSSPaddingBottom(), containingBlock->ComputedCSSPaddingLeft()});
    }

    LayoutUnit maxWidth = containingBlock->ContentBoxLogicalWidth();

    const auto &zoomFactor = Style().UsedZoomForLength();

    // Sticky positioned element ignore any override logical width on the containing block (as they don't call
    // containingBlockLogicalWidthForContent). It's unclear whether this is totally fine.
    LayoutBoxExtent minMargin(
      style::EvaluateMinimum<LayoutUnit>(Style().MarginTop(), maxWidth, zoomFactor),
      style::EvaluateMinimum<LayoutUnit>(Style().MarginRight(), maxWidth, zoomFactor),
      style::EvaluateMinimum<LayoutUnit>(Style().MarginBottom(), maxWidth, zoomFactor),
      style::EvaluateMinimum<LayoutUnit>(Style().MarginLeft(), maxWidth, zoomFactor));

    // Compute the container-relative area within which the sticky element is allowed to move.
    containerContentRect.Contract(minMargin);

    // Finally compute container rect relative to the scrolling ancestor. We pass an empty mode here, because
    // sticky positioning should ignore transforms.
    FloatRect containerRectRelativeToScrollingAncestor =
      containingBlock->LocalToContainerQuad(FloatRect(containerContentRect), &enclosingClippingBox, {})
        .BoundingBox();
    if (enclosingClippingLayer)
    {
      FloatPoint containerLocationRelativeToScrollingAncestor =
        containerRectRelativeToScrollingAncestor.Location()
        - FloatSize(enclosingClippingBox.BorderLeft() + enclosingClippingBox.PaddingLeft(),
                    enclosingClippingBox.BorderTop() + enclosingClippingBox.PaddingTop());
      if (&enclosingClippingBox != containingBlock)
      {
        if (auto *scrollableArea = enclosingClippingLayer->GetScrollableArea())
        {
          containerLocationRelativeToScrollingAncestor +=
            FloatSize(ToIntSize(scrollableArea->GetScrollOffset()));
        }
      }
      containerRectRelativeToScrollingAncestor.SetLocation(containerLocationRelativeToScrollingAncestor);
    }
    constraints.SetContainingBlockRect(containerRectRelativeToScrollingAncestor);

    // Now compute the sticky box rect, also relative to the scrolling ancestor.
    LayoutRect stickyBoxRect = FrameRectForStickyPositioning();

    // Ideally, it would be possible to call this->localToContainerQuad to determine the frame rectangle in
    // the coordinate system of the scrolling ancestor, but localToContainerQuad itself depends on sticky
    // positioning! Instead, start from the parent but first adjusting the rectangle for the writing mode of
    // this stickily-positioned element. We also pass an empty mode here because sticky positioning should
    // ignore transforms.
    //
    // FIXME: It would also be nice to not have to call localToContainerQuad again since we have already done
    // a similar call to move from the containing block to the scrolling ancestor above, but
    // localToContainerQuad takes care of a lot of complex situations involving inlines, tables, and
    // transformations.
    if (auto *parentBox = DynamicDowncast<RenderBox>(*Parent()))
    {
      parentBox->FlipForWritingMode(stickyBoxRect);
    }
    auto stickyBoxRelativeToScrollingAncestor =
      Parent()->LocalToContainerQuad(FloatRect(stickyBoxRect), &enclosingClippingBox, {}).BoundingBox();

    if (enclosingClippingLayer)
    {
      stickyBoxRelativeToScrollingAncestor.Move(
        -FloatSize(enclosingClippingBox.BorderLeft() + enclosingClippingBox.PaddingLeft(),
                   enclosingClippingBox.BorderTop() + enclosingClippingBox.PaddingTop()));

      if (&enclosingClippingBox != Parent())
      {
        if (auto *scrollableArea = enclosingClippingLayer->GetScrollableArea())
        {
          stickyBoxRelativeToScrollingAncestor.Move(FloatSize(ToIntSize(scrollableArea->GetScrollOffset())));
        }
      }
    }
    constraints.SetStickyBoxRect(stickyBoxRelativeToScrollingAncestor);

    if (!Style().Left().IsAuto())
    {
      constraints.SetLeftOffset(style::Evaluate<float>(Style().Left(), constrainingRect.Width(), zoomFactor));
      constraints.AddAnchorEdge(AnchorEdge::Left);
    }

    if (!Style().Right().IsAuto())
    {
      constraints.SetRightOffset(
        style::Evaluate<float>(Style().Right(), constrainingRect.Width(), zoomFactor));
      constraints.AddAnchorEdge(AnchorEdge::Right);
    }

    if (!Style().Top().IsAuto())
    {
      constraints.SetTopOffset(style::Evaluate<float>(Style().Top(), constrainingRect.Height(), zoomFactor));
      constraints.AddAnchorEdge(AnchorEdge::Top);
    }

    if (!Style().Bottom().IsAuto())
    {
      constraints.SetBottomOffset(
        style::Evaluate<float>(Style().Bottom(), constrainingRect.Height(), zoomFactor));
      constraints.AddAnchorEdge(AnchorEdge::Bottom);
    }

    if (constraints.HasAnchorEdge(AnchorEdge::Right) && constraints.HasAnchorEdge(AnchorEdge::Left))
    {
      float availableSpace = constrainingRect.Width() - constraints.LeftOffset() - constraints.RightOffset();
      if (constraints.StickyBoxRect().Width() > availableSpace)
      {
        float delta = constraints.StickyBoxRect().Width() - availableSpace;
        if (Style().GetWritingMode().IsAnyLeftToRight())
        {
          constraints.SetRightOffset(constraints.RightOffset() - delta);
        }
        else
        {
          constraints.SetLeftOffset(constraints.LeftOffset() - delta);
        }
      }
    }

    if (constraints.HasAnchorEdge(AnchorEdge::Bottom) && constraints.HasAnchorEdge(AnchorEdge::Top))
    {
      float availableSpace = constrainingRect.Height() - constraints.TopOffset() - constraints.BottomOffset();
      if (constraints.StickyBoxRect().Height() > availableSpace)
      {
        float delta = constraints.StickyBoxRect().Height() - availableSpace;
        if (Style().GetWritingMode().IsAnyTopToBottom())
        {
          constraints.SetBottomOffset(constraints.BottomOffset() - delta);
        }
        else
        {
          constraints.SetTopOffset(constraints.TopOffset() - delta);
        }
      }
    }
  }

  FloatRect RenderBoxModelObject::ConstrainingRectForStickyPosition() const noexcept
  {
    RenderLayer *enclosingClippingLayer =
      HasLayer() ? Layer()->EnclosingOverflowClipLayer(ExcludeSelf) : nullptr;

    if (enclosingClippingLayer)
    {
      auto &enclosingClippingBox = Downcast<RenderBox>(enclosingClippingLayer->Renderer());
      LayoutRect clipRect = enclosingClippingBox.OverflowClipRect({});
      clipRect.Contract(LayoutSize(enclosingClippingBox.PaddingLeft() + enclosingClippingBox.PaddingRight(),
                                   enclosingClippingBox.PaddingTop() + enclosingClippingBox.PaddingBottom()));

      FloatRect constrainingRect =
        enclosingClippingBox.LocalToContainerQuad(FloatRect(clipRect), &View()).BoundingBox();

      auto *scrollableArea = enclosingClippingLayer->GetScrollableArea();
      FloatPoint scrollOffset;
      if (scrollableArea)
      {
        scrollOffset = FloatPoint() + FloatSize(ToIntSize(scrollableArea->GetScrollOffset()));
      }

      float scrollbarOffset = 0;
      if (scrollableArea && enclosingClippingBox.HasLayer()
          && scrollableArea->ShouldPlaceVerticalScrollbarOnLeft())
      {
        scrollbarOffset = static_cast<float>(
          scrollableArea->VerticalScrollbarWidth(OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize,
                                                 Style().GetWritingMode().IsHorizontal()));
      }

      constrainingRect.SetLocation(FloatPoint(scrollOffset.X() + scrollbarOffset, scrollOffset.Y()));
      return constrainingRect;
    }

    return FloatRect(View().FrameView().RectForFixedPositionLayout());
  }

  LayoutSize RenderBoxModelObject::StickyPositionOffset() const noexcept
  {
    FloatRect constrainingRect = ConstrainingRectForStickyPosition();
    StickyPositionViewportConstraints constraints;
    ComputeStickyPositionConstraints(constraints, constrainingRect);

    // The sticky offset is physical, so we can just return the delta computed in absolute coords (though it
    // may be wrong with transforms).
    return LayoutSize(constraints.ComputeStickyOffset(constrainingRect));
  }

  LayoutPoint
    RenderBoxModelObject::AdjustedPositionRelativeToOffsetParent(const LayoutPoint &startPoint) const noexcept
  {
    // If the element is the HTML body element or doesn't have a parent
    // return 0 and stop this algorithm.
    if (IsBody() || !Parent())
    {
      return {};
    }

    LayoutPoint referencePoint = startPoint;

    // If the offsetParent of the element is null, or is the HTML body element,
    // return the distance between the canvas origin and the left border edge
    // of the element and stop this algorithm.
    if (const auto *offsetParent = OffsetParent())
    {
      if (auto *renderBox = DynamicDowncast<RenderBox>(*offsetParent);
          renderBox && !offsetParent->IsBody() && !Is<RenderTable>(*offsetParent))
      {
        referencePoint.Move(-renderBox->BorderLeft(), -renderBox->BorderTop());
      }
      // FIXME: Inside an inline formatting context, the position of the inline offset parent's first inline
      // box (firstInlineBoxTopLeft), once there's inline layout.

      // FIXME: Also when the renderer is in a fragmented flow, once there are fragmented flows.
      if (!IsOutOfFlowPositioned())
      {
        if (IsRelativelyPositioned())
        {
          referencePoint.Move(RelativePositionOffset());
        }
        else if (IsStickilyPositioned())
        {
          referencePoint.Move(StickyPositionOffset());
        }

        auto *ancestor = Parent();
        while (ancestor != offsetParent)
        {
          // FIXME: What are we supposed to do inside SVG content?
          // FIXME: The translation of a multi-column flow, once there are multi-column flows.
          if (auto *renderBox = DynamicDowncast<RenderBox>(*ancestor);
              renderBox && !Is<RenderTableRow>(*ancestor))
          {
            referencePoint.MoveBy(renderBox->TopLeftLocation());
          }

          ancestor = ancestor->Parent();
        }

        if (auto *renderBox = DynamicDowncast<RenderBox>(*offsetParent);
            renderBox && offsetParent->IsBody() && !offsetParent->IsPositioned())
        {
          referencePoint.MoveBy(renderBox->TopLeftLocation());
        }
      }
    }

    return referencePoint;
  }

  LayoutUnit RenderBoxModelObject::OffsetLeft() const noexcept
  {
    // Note that RenderInline and RenderBox override this to pass a different
    // startPoint to adjustedPositionRelativeToOffsetParent.
    return AdjustedPositionRelativeToOffsetParent(LayoutPoint()).X();
  }

  LayoutUnit RenderBoxModelObject::OffsetTop() const noexcept
  {
    // Note that RenderInline and RenderBox override this to pass a different
    // startPoint to adjustedPositionRelativeToOffsetParent.
    return AdjustedPositionRelativeToOffsetParent(LayoutPoint()).Y();
  }

#pragma endregion

  bool RenderBoxModelObject::RequiresLayer() const noexcept
  {
    // FIXME: Reflections were dropped; view transition captures, once there are view transitions.
    return IsDocumentElementRenderer() || IsPositioned() || CreatesGroup() || HasTransformRelatedProperty()
           || HasHiddenBackface() || RequiresRenderingConsolidationForViewTransition();
  }

  void RenderBoxModelObject::ApplyTransform(TransformationMatrix &, const RenderStyle &, const FloatRect &,
                                            EnumSet<style::TransformResolverOption>) const noexcept
  {
    // applyTransform() is only used through RenderLayer*, which only invokes this for RenderBox derived
    // renderers, thus not for RenderInline/RenderLineBreak - the other two renderers that inherit from
    // RenderBoxModelObject.
    krys_debug_assert(false);
  }

  void RenderBoxModelObject::RemoveOutOfFlowBoxesIfNeededOnStyleChange(RenderBlock &delegateBlock,
                                                                       const RenderStyle &oldStyle,
                                                                       const RenderStyle &newStyle) noexcept
  {
    auto wasContainingBlockForFixedContent = CanContainFixedPositionObjects(&oldStyle);
    auto wasContainingBlockForAbsoluteContent = CanContainAbsolutelyPositionedObjects(&oldStyle);
    auto isContainingBlockForFixedContent = CanContainFixedPositionObjects(&newStyle);
    auto isContainingBlockForAbsoluteContent = CanContainAbsolutelyPositionedObjects(&newStyle);

    // FIXME: If an inline becomes a containing block, but the delegate was already one (or vice-versa),
    // then we don't really need to remove the out-of-flows from the delegate only for them to be re-added
    // to the same spot. We would need to correctly mark for layout instead though.

    if ((wasContainingBlockForFixedContent && !isContainingBlockForFixedContent)
        || (wasContainingBlockForAbsoluteContent && !isContainingBlockForAbsoluteContent))
    {
      // We are no longer the containing block for out-of-flow descendants.
      delegateBlock.RemoveOutOfFlowBoxes({}, RenderBlock::ContainingBlockState::NewContainingBlock);
    }

    if (!wasContainingBlockForFixedContent && isContainingBlockForFixedContent)
    {
      // We are a new containing block for all out-of-flow boxes. Find first ancestor that has our fixed
      // positioned boxes and remove them. They will be inserted into our positioned objects list during their
      // static position layout.
      if (auto *containingBlock = RenderObject::ContainingBlockForPositionType(PositionType::Fixed, *this))
      {
        containingBlock->RemoveOutOfFlowBoxes(&delegateBlock,
                                              RenderBlock::ContainingBlockState::NewContainingBlock);
      }
    }

    if (!wasContainingBlockForAbsoluteContent && isContainingBlockForAbsoluteContent)
    {
      // We are a new containing block for absolute positioning.
      // Remove our absolutely positioned descendants from their current containing block.
      // They will be inserted into our positioned objects list during layout.
      if (auto *containingBlock = RenderObject::ContainingBlockForPositionType(PositionType::Absolute, *this))
      {
        containingBlock->RemoveOutOfFlowBoxes(&delegateBlock,
                                              RenderBlock::ContainingBlockState::NewContainingBlock);
      }
    }
  }

  namespace
  {
    KRYS_NODISCARD LayoutUnit ResolveWidthForRatio(LayoutUnit height,
                                                   const LayoutSize &intrinsicRatio) noexcept
    {
      return height * intrinsicRatio.Width() / intrinsicRatio.Height();
    }

    KRYS_NODISCARD LayoutUnit ResolveHeightForRatio(LayoutUnit width,
                                                    const LayoutSize &intrinsicRatio) noexcept
    {
      return width * intrinsicRatio.Height() / intrinsicRatio.Width();
    }

    KRYS_NODISCARD LayoutSize ResolveAgainstIntrinsicWidthOrHeightAndRatio(const LayoutSize &size,
                                                                           const LayoutSize &intrinsicRatio,
                                                                           LayoutUnit useWidth,
                                                                           LayoutUnit useHeight) noexcept
    {
      if (intrinsicRatio.IsEmpty())
      {
        if (useWidth)
        {
          return LayoutSize(useWidth, size.Height());
        }
        return LayoutSize(size.Width(), useHeight);
      }

      if (useWidth)
      {
        return LayoutSize(useWidth, ResolveHeightForRatio(useWidth, intrinsicRatio));
      }
      return LayoutSize(ResolveWidthForRatio(useHeight, intrinsicRatio), useHeight);
    }

    KRYS_NODISCARD LayoutSize ResolveAgainstIntrinsicRatio(const LayoutSize &size,
                                                           const LayoutSize &intrinsicRatio) noexcept
    {
      // Two possible solutions: (size.width(), solutionHeight) or (solutionWidth, size.height())
      // "... must be assumed to be the largest dimensions..." = easiest answer: the rect with the largest
      // surface area.

      LayoutUnit solutionWidth = ResolveWidthForRatio(size.Height(), intrinsicRatio);
      LayoutUnit solutionHeight = ResolveHeightForRatio(size.Width(), intrinsicRatio);
      if (solutionWidth <= size.Width())
      {
        if (solutionHeight <= size.Height())
        {
          // If both solutions fit, choose the one covering the larger area.
          LayoutUnit areaOne = solutionWidth * size.Height();
          LayoutUnit areaTwo = size.Width() * solutionHeight;
          if (areaOne < areaTwo)
          {
            return LayoutSize(size.Width(), solutionHeight);
          }
          return LayoutSize(solutionWidth, size.Height());
        }

        // Only the first solution fits.
        return LayoutSize(solutionWidth, size.Height());
      }

      // Only the second solution fits, assert that.
      krys_debug_assert(solutionHeight <= size.Height());
      return LayoutSize(size.Width(), solutionHeight);
    }
  }

  LayoutSize RenderBoxModelObject::CalculateImageIntrinsicDimensions(
    StyleImage *image, const LayoutSize &positioningAreaSize, ScaleByUsedZoom scaleByUsedZoom) const noexcept
  {
    // A generated image without a fixed size, will always return the container size as intrinsic size.
    if (!image->ImageHasNaturalDimensions())
    {
      return LayoutSize(positioningAreaSize.Width(), positioningAreaSize.Height());
    }

    float intrinsicWidth = 0;
    float intrinsicHeight = 0;
    FloatSize intrinsicRatio;
    image->ComputeIntrinsicDimensions(this, intrinsicWidth, intrinsicHeight, intrinsicRatio);

    LayoutSize resolvedSize(intrinsicWidth, intrinsicHeight);
    LayoutSize minimumSize(resolvedSize.Width() > 0 ? 1 : 0, resolvedSize.Height() > 0 ? 1 : 0);

    if (scaleByUsedZoom == ScaleByUsedZoom::Yes)
    {
      resolvedSize.Scale(Style().UsedZoom());
    }
    resolvedSize.ClampToMinimumSize(minimumSize);

    if (!resolvedSize.IsEmpty())
    {
      return resolvedSize;
    }

    // If the image has one of either an intrinsic width or an intrinsic height:
    // * and an intrinsic aspect ratio, then the missing dimension is calculated from the given dimension and
    // the ratio.
    // * and no intrinsic aspect ratio, then the missing dimension is assumed to be the size of the rectangle
    // that
    //   establishes the coordinate system for the 'background-position' property.
    if (resolvedSize.Width() > 0 || resolvedSize.Height() > 0)
    {
      return ResolveAgainstIntrinsicWidthOrHeightAndRatio(positioningAreaSize, LayoutSize(intrinsicRatio),
                                                          resolvedSize.Width(), resolvedSize.Height());
    }

    // If the image has no intrinsic dimensions and has an intrinsic ratio the dimensions must be assumed to
    // be the largest dimensions at that ratio such that neither dimension exceeds the dimensions of the
    // rectangle that establishes the coordinate system for the 'background-position' property.
    if (!intrinsicRatio.IsEmpty())
    {
      return ResolveAgainstIntrinsicRatio(positioningAreaSize, LayoutSize(intrinsicRatio));
    }

    // If the image has no intrinsic ratio either, then the dimensions must be assumed to be the rectangle
    // that establishes the coordinate system for the 'background-position' property.
    return positioningAreaSize;
  }

  bool RenderBoxModelObject::FixedBackgroundPaintsInLocalCoordinates() const noexcept
  {
    // Only a composited root layer paints its fixed background in its own coordinates; there's no
    // compositing.
    return false;
  }

  InterpolationQuality RenderBoxModelObject::ChooseInterpolationQuality(GraphicsContext &context,
                                                                        Image &image, const void *,
                                                                        const LayoutSize &) const noexcept
  {
    return ImageQualityController::ChooseInterpolationQuality(context, *this, image);
  }

  bool RenderBoxModelObject::BorderObscuresBackgroundEdge(const FloatSize &contextScale) const noexcept
  {
    auto edges = GetBorderEdges(Style(), GetDocument().DeviceScaleFactor());

    for (auto side : AllBoxSides)
    {
      auto &currEdge = edges.At(side);
      // FIXME: for vertical text
      float axisScale =
        (side == BoxSide::Top || side == BoxSide::Bottom) ? contextScale.Height() : contextScale.Width();
      if (!currEdge.ObscuresBackgroundEdge(axisScale))
      {
        return false;
      }
    }

    return true;
  }

  bool RenderBoxModelObject::BorderObscuresBackground() const noexcept
  {
    if (!Style().HasBorder())
    {
      return false;
    }

    // Bail if we have any border-image for now. We could look at the image alpha to improve this.
    if (!Style().GetBorderImageSource().IsNone())
    {
      return false;
    }

    auto edges = GetBorderEdges(Style(), GetDocument().DeviceScaleFactor());

    for (auto side : AllBoxSides)
    {
      if (!edges.At(side).ObscuresBackground())
      {
        return false;
      }
    }

    return true;
  }

  BorderShape RenderBoxModelObject::BorderShapeForContentClipping(const LayoutRect &borderBoxRect,
                                                                  RectEdges<bool> closedEdges) const noexcept
  {
    auto borderWidths = this->BorderWidths();
    auto padding = this->Padding();

    auto contentBoxInsets = RectEdges<LayoutUnit> {
      borderWidths.Top() + padding.Top(),
      borderWidths.Right() + padding.Right(),
      borderWidths.Bottom() + padding.Bottom(),
      borderWidths.Left() + padding.Left(),
    };

    return BorderShape::ShapeForBorderRect(Style(), borderBoxRect, contentBoxInsets, closedEdges);
  }
}
