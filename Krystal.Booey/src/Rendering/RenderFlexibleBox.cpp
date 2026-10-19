#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Graphics/FontBaseline.hpp"
#include "Krystal.Booey/Layout/Integration/Flex/LayoutIntegrationFlexLayout.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/BaselineAlignment.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderReplaced.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleContentAlignmentData.hpp"
#include "Krystal.Booey/Rendering/Style/StyleSelfAlignmentData.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include <cmath>
#include <limits>

namespace krys::boo
{
  RenderFlexibleBox::FlexLayoutItem::FlexLayoutItem(RenderBox &flexItem, LayoutUnit flexBaseContentSize,
                                                    LayoutUnit mainAxisBorderAndPadding,
                                                    LayoutUnit mainAxisMargin,
                                                    std::pair<LayoutUnit, LayoutUnit> minMaxSizes,
                                                    bool everHadLayout) noexcept
      : Renderer(&flexItem), FlexBaseContentSize(flexBaseContentSize),
        MainAxisBorderAndPadding(mainAxisBorderAndPadding), MainAxisMargin(mainAxisMargin),
        MinMaxSizes(minMaxSizes), HypotheticalMainContentSize(ConstrainSizeByMinMax(flexBaseContentSize)),
        Frozen(false), EverHadLayout(everHadLayout)
  {
    krys_debug_assert(!flexItem.IsOutOfFlowPositioned());
  }

  LayoutUnit RenderFlexibleBox::FlexLayoutItem::HypotheticalMainAxisMarginBoxSize() const noexcept
  {
    return HypotheticalMainContentSize + MainAxisBorderAndPadding + MainAxisMargin;
  }

  LayoutUnit RenderFlexibleBox::FlexLayoutItem::FlexBaseMarginBoxSize() const noexcept
  {
    return FlexBaseContentSize + MainAxisBorderAndPadding + MainAxisMargin;
  }

  LayoutUnit RenderFlexibleBox::FlexLayoutItem::FlexedMarginBoxSize() const noexcept
  {
    return FlexedContentSize + MainAxisBorderAndPadding + MainAxisMargin;
  }

  const RenderStyle &RenderFlexibleBox::FlexLayoutItem::Style() const noexcept
  {
    return Renderer->Style();
  }

  LayoutUnit RenderFlexibleBox::FlexLayoutItem::ConstrainSizeByMinMax(LayoutUnit size) const noexcept
  {
    return std::max(MinMaxSizes.first, std::min(size, MinMaxSizes.second));
  }

  struct RenderFlexibleBox::LineState
  {
    LineState(LayoutUnit crossAxisOffset, LayoutUnit crossAxisExtent,
              Maybe<BaselineAlignmentState> baselineAlignmentState,
              FlexLayoutItems &&flexLayoutItems) noexcept
        : CrossAxisOffset(crossAxisOffset), CrossAxisExtent(crossAxisExtent),
          BaselineAlignment(krys::move(baselineAlignmentState)), FlexLayoutItems(krys::move(flexLayoutItems))
    {
    }

    LayoutUnit CrossAxisOffset;
    LayoutUnit CrossAxisExtent;
    Maybe<BaselineAlignmentState> BaselineAlignment;
    RenderFlexibleBox::FlexLayoutItems FlexLayoutItems;
  };

  namespace
  {
    // RAII class which defines a scope in which overriding sizes of a box are either:
    //   1) replaced by other size in one axis if size is specified
    //   2) cleared in both axis if size == std::nullopt
    //
    // In any case the previous overriding sizes are restored on destruction (in case of
    // not having a previous value it's simply cleared).
    class OverridingSizesScope
    {
      KRYS_NON_COPY_MOVABLE(OverridingSizesScope);

    public:
      enum class Axis : uint8
      {
        Inline,
        Block,
        Both
      };

    private:
      RenderBox &_box;
      Axis _axis;
      Maybe<LayoutUnit> _previousOverridingBorderBoxLogicalWidth;
      Maybe<LayoutUnit> _previousOverridingBorderBoxLogicalHeight;

      static void SetOrClearWidth(RenderBox &box, Maybe<LayoutUnit> size) noexcept
      {
        if (size)
        {
          box.SetOverridingBorderBoxLogicalWidth(*size);
        }
        else
        {
          box.ClearOverridingBorderBoxLogicalWidth();
        }
      }
      static void SetOrClearHeight(RenderBox &box, Maybe<LayoutUnit> size) noexcept
      {
        if (size)
        {
          box.SetOverridingBorderBoxLogicalHeight(*size);
        }
        else
        {
          box.ClearOverridingBorderBoxLogicalHeight();
        }
      }

    public:
      OverridingSizesScope(RenderBox &box, Axis axis, Maybe<LayoutUnit> size = null) noexcept
          : _box(box), _axis(axis)
      {
        krys_debug_assert(!size || (axis != Axis::Both));
        if (axis == Axis::Both || axis == Axis::Inline)
        {
          _previousOverridingBorderBoxLogicalWidth = box.OverridingBorderBoxLogicalWidth();
          SetOrClearWidth(_box, size);
        }
        if (axis == Axis::Both || axis == Axis::Block)
        {
          _previousOverridingBorderBoxLogicalHeight = box.OverridingBorderBoxLogicalHeight();
          SetOrClearHeight(_box, size);
        }
      }
      ~OverridingSizesScope() noexcept
      {
        if (_axis == Axis::Inline || _axis == Axis::Both)
        {
          SetOrClearWidth(_box, _previousOverridingBorderBoxLogicalWidth);
        }

        if (_axis == Axis::Block || _axis == Axis::Both)
        {
          SetOrClearHeight(_box, _previousOverridingBorderBoxLogicalHeight);
        }
      }
    };

    void UpdateFlexItemDirtyBitsBeforeLayout(bool relayoutFlexItem, RenderBox &flexItem) noexcept
    {
      if (flexItem.IsOutOfFlowPositioned())
      {
        return;
      }

      // FIXME: Technically percentage height objects only need a relayout if their percentage isn't going to
      // be turned into an auto value. Add a method to determine this, so that we can avoid the relayout.
      if (relayoutFlexItem || flexItem.HasRelativeLogicalHeight())
      {
        flexItem.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }
    }

    KRYS_NODISCARD const StyleContentAlignmentData &ContentAlignmentNormalBehavior() noexcept
    {
      // The justify-content property applies along the main axis, but since
      // flexing in the main axis is controlled by flex, stretch behaves as
      // flex-start (ignoring the specified fallback alignment, if any).
      // https://drafts.csswg.org/css-align/#distribution-flex
      static const StyleContentAlignmentData normalBehavior = {ContentPosition::Normal,
                                                               ContentDistribution::Stretch};
      return normalBehavior;
    }

    // FIXME: SVG roots, which may have an intrinsic aspect ratio but no intrinsic size, once there's SVG
    // (isSVGRootWithIntrinsicAspectRatio).
    KRYS_NODISCARD bool IsSVGRootWithIntrinsicAspectRatio(const RenderBox &) noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool FlexItemHasAspectRatio(const RenderBox &flexItem) noexcept
    {
      return flexItem.HasIntrinsicAspectRatio() || flexItem.Style().HasAspectRatio()
             || IsSVGRootWithIntrinsicAspectRatio(flexItem);
    }

    // This is a RAII class that is used to temporarily set the flex basis as the child size in the main axis.
    class ScopedFlexBasisAsFlexItemMainSize
    {
      KRYS_NON_COPY_MOVABLE(ScopedFlexBasisAsFlexItemMainSize);

    private:
      RenderBox &_flexItem;
      bool _mainAxisIsInlineAxis {true};
      bool _didOverride {false};

    public:
      ScopedFlexBasisAsFlexItemMainSize(RenderBox &flexItem, style::PreferredSize &&flexBasis,
                                        bool mainAxisIsInlineAxis) noexcept
          : _flexItem(flexItem), _mainAxisIsInlineAxis(mainAxisIsInlineAxis)
      {
        if (flexBasis.IsAuto())
        {
          return;
        }

        if (_mainAxisIsInlineAxis)
        {
          _flexItem.SetOverridingBorderBoxLogicalWidthForFlexBasisComputation(krys::move(flexBasis));
        }
        else
        {
          _flexItem.SetOverridingBorderBoxLogicalHeightForFlexBasisComputation(krys::move(flexBasis));
        }
        _didOverride = true;
      }

      ~ScopedFlexBasisAsFlexItemMainSize() noexcept
      {
        if (!_didOverride)
        {
          return;
        }

        if (_mainAxisIsInlineAxis)
        {
          _flexItem.ClearOverridingLogicalWidthForFlexBasisComputation();
        }
        else
        {
          _flexItem.ClearOverridingLogicalHeightForFlexBasisComputation();
        }
      }
    };

    KRYS_NODISCARD inline ContentPosition ResolveLeftRightAlignment(ContentPosition position,
                                                                    StyleContentAlignmentData justifyContent,
                                                                    const RenderStyle &style,
                                                                    bool isReversed) noexcept
    {
      if (position == ContentPosition::Left || position == ContentPosition::Right)
      {
        auto leftRightAxisDirection = RenderFlexibleBox::LeftRightAxisDirectionFromStyle(style);
        position = justifyContent.IsEndward(leftRightAxisDirection, isReversed) ? ContentPosition::End
                                                                                : ContentPosition::Start;
      }
      return position;
    }

    KRYS_NODISCARD LayoutUnit InitialJustifyContentOffset(const RenderStyle &style,
                                                          LayoutUnit availableFreeSpace,
                                                          uint32 numberOfFlexItems, bool isReversed) noexcept
    {
      auto resolvedJustifyContent = style.GetJustifyContent().Resolve(ContentAlignmentNormalBehavior());
      auto justifyContentPosition = resolvedJustifyContent.Position();
      auto justifyContentDistribution = resolvedJustifyContent.Distribution();

      if (availableFreeSpace < 0 && resolvedJustifyContent.Overflow() == OverflowAlignment::Safe)
      {
        krys_debug_assert(justifyContentPosition != ContentPosition::Normal);
        justifyContentPosition = ContentPosition::Start;
      }
      else
      {
        // First of all resolve Left and Right so we could convert it to their equivalent properties handled
        // bellow. If the property's axis is not parallel with either left<->right axis, this value behaves as
        // start. Currently, the only case where the property's axis is not parallel with either left<->right
        // axis is in a column flexbox. https: //www.w3.org/TR/css-align-3/#valdef-justify-content-left
        justifyContentPosition =
          ResolveLeftRightAlignment(justifyContentPosition, resolvedJustifyContent, style, isReversed);
      }

      krys_debug_assert(justifyContentPosition != ContentPosition::Left);
      krys_debug_assert(justifyContentPosition != ContentPosition::Right);

      if (justifyContentPosition == ContentPosition::FlexEnd
          || (justifyContentPosition == ContentPosition::End && !isReversed)
          || (justifyContentPosition == ContentPosition::Start && isReversed))
      {
        return availableFreeSpace;
      }
      if (justifyContentPosition == ContentPosition::Center)
      {
        return availableFreeSpace / 2;
      }
      if (justifyContentDistribution == ContentDistribution::SpaceAround)
      {
        if (!numberOfFlexItems)
        {
          return availableFreeSpace / 2;
        }
        if (availableFreeSpace > 0)
        {
          return availableFreeSpace / (2 * numberOfFlexItems);
        }
        return {};
      }
      if (justifyContentDistribution == ContentDistribution::SpaceEvenly)
      {
        if (!numberOfFlexItems)
        {
          return availableFreeSpace / 2;
        }
        if (availableFreeSpace > 0)
        {
          return availableFreeSpace / (numberOfFlexItems + 1);
        }
        return {};
      }
      return {};
    }

    KRYS_NODISCARD LayoutUnit JustifyContentSpaceBetweenFlexItems(
      LayoutUnit availableFreeSpace, ContentDistribution justifyContentDistribution,
      uint32 numberOfFlexItems) noexcept
    {
      if (availableFreeSpace > 0 && numberOfFlexItems > 1)
      {
        if (justifyContentDistribution == ContentDistribution::SpaceBetween)
        {
          return availableFreeSpace / (numberOfFlexItems - 1);
        }
        if (justifyContentDistribution == ContentDistribution::SpaceAround)
        {
          return availableFreeSpace / numberOfFlexItems;
        }
        if (justifyContentDistribution == ContentDistribution::SpaceEvenly)
        {
          return availableFreeSpace / (numberOfFlexItems + 1);
        }
      }
      return 0_lu;
    }

    KRYS_NODISCARD LayoutUnit AlignmentOffset(LayoutUnit availableFreeSpace, ItemPosition position,
                                              Maybe<LayoutUnit> ascent, Maybe<LayoutUnit> maxAscent,
                                              bool isWrapReverse) noexcept
    {
      switch (position)
      {
        case ItemPosition::Legacy:
        case ItemPosition::Auto:
        case ItemPosition::Normal: krys_debug_assert(false); break;
        case ItemPosition::Start:
        case ItemPosition::End:
        case ItemPosition::SelfStart:
        case ItemPosition::SelfEnd:
        case ItemPosition::Left:
        case ItemPosition::Right:
          // AlignmentForFlexItem should have transformed this position value to something we handle below.
          krys_debug_assert(false);
          break;
        case ItemPosition::Stretch:
          // Actual stretching must be handled by the caller. Since wrap-reverse
          // flips cross start and cross end, stretch children should be aligned
          // with the cross end. This matters because applyStretchAlignment
          // doesn't always stretch or stretch fully (explicit cross size given, or
          // stretching constrained by max-height/max-width). For flex-start and
          // flex-end this is handled by alignmentForFlexItem().
          if (isWrapReverse)
          {
            return availableFreeSpace;
          }
          break;
        case ItemPosition::FlexStart:    break;
        case ItemPosition::FlexEnd:      return availableFreeSpace;
        case ItemPosition::Center:
        case ItemPosition::AnchorCenter: return availableFreeSpace / 2;
        case ItemPosition::Baseline:
        case ItemPosition::LastBaseline: return maxAscent.value_or(0_lu) - ascent.value_or(0_lu);
      }
      return 0_lu;
    }

    KRYS_NODISCARD LayoutUnit ContentAlignmentStartOverflow(LayoutUnit availableFreeSpace,
                                                            ContentPosition position,
                                                            ContentDistribution distribution,
                                                            OverflowAlignment safety, bool isReverse) noexcept
    {
      if (availableFreeSpace >= 0 || safety == OverflowAlignment::Safe)
      {
        return 0_lu;
      }

      if (distribution == ContentDistribution::SpaceAround
          || distribution == ContentDistribution::SpaceEvenly)
      {
        return -availableFreeSpace / 2;
      }

      switch (position)
      {
        case ContentPosition::Start:
        case ContentPosition::Baseline:
        case ContentPosition::LastBaseline: return 0_lu;
        case ContentPosition::FlexStart:    return isReverse ? -availableFreeSpace : 0_lu;
        case ContentPosition::Center:       return -availableFreeSpace / 2;
        case ContentPosition::End:          return -availableFreeSpace;
        case ContentPosition::FlexEnd:      return isReverse ? 0_lu : -availableFreeSpace;
        default:
          krys_debug_assert((distribution == ContentDistribution::Default
                             && position == ContentPosition::Normal) // Normal alignment.
                            || distribution == ContentDistribution::Stretch
                            || distribution == ContentDistribution::SpaceBetween);
          return isReverse ? -availableFreeSpace : 0_lu;
      }
    }

    KRYS_NODISCARD LayoutUnit InitialAlignContentOffset(LayoutUnit availableFreeSpace,
                                                        ContentPosition alignContent,
                                                        ContentDistribution alignContentDistribution,
                                                        OverflowAlignment safety, uint32 numberOfLines,
                                                        bool isReversed) noexcept
    {
      if (availableFreeSpace < 0 && safety == OverflowAlignment::Safe)
      {
        krys_debug_assert(alignContent != ContentPosition::Normal);
        alignContent = ContentPosition::Start;
      }

      if (alignContent == ContentPosition::FlexEnd || (alignContent == ContentPosition::End && !isReversed)
          || (alignContent == ContentPosition::Start && isReversed))
      {
        return availableFreeSpace;
      }
      if (alignContent == ContentPosition::Center)
      {
        return availableFreeSpace / 2;
      }
      if (alignContentDistribution == ContentDistribution::SpaceAround)
      {
        if (availableFreeSpace > 0 && numberOfLines)
        {
          return availableFreeSpace / (2 * numberOfLines);
        }
        if (availableFreeSpace < 0)
        {
          return std::max(0_lu, availableFreeSpace / 2);
        }
      }
      if (alignContentDistribution == ContentDistribution::SpaceEvenly)
      {
        if (availableFreeSpace > 0)
        {
          return availableFreeSpace / (numberOfLines + 1);
        }
        // Fallback to 'safe center'
        return std::max(0_lu, availableFreeSpace / 2);
      }
      return 0_lu;
    }

    KRYS_NODISCARD LayoutUnit AlignContentSpaceBetweenFlexItems(LayoutUnit availableFreeSpace,
                                                                ContentDistribution alignContentDistribution,
                                                                uint32 numberOfLines) noexcept
    {
      if (availableFreeSpace > 0 && numberOfLines > 1)
      {
        if (alignContentDistribution == ContentDistribution::SpaceBetween)
        {
          return availableFreeSpace / (numberOfLines - 1);
        }
        if (alignContentDistribution == ContentDistribution::SpaceAround
            || alignContentDistribution == ContentDistribution::Stretch)
        {
          return availableFreeSpace / numberOfLines;
        }
        if (alignContentDistribution == ContentDistribution::SpaceEvenly)
        {
          return availableFreeSpace / (numberOfLines + 1);
        }
      }
      return 0_lu;
    }
  }

  RenderFlexibleBox::RenderFlexibleBox(Type type, dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlock(type, element, krys::move(style), TypeFlag::IsFlexibleBox)
  {
    krys_debug_assert(IsRenderFlexibleBox());
    SetChildrenInline(false); // All of our children must be block-level.
  }

  RenderFlexibleBox::RenderFlexibleBox(Type type, dom::Document &document, RenderStyle &&style) noexcept
      : RenderBlock(type, document, krys::move(style), TypeFlag::IsFlexibleBox)
  {
    krys_debug_assert(IsRenderFlexibleBox());
    SetChildrenInline(false); // All of our children must be block-level.
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderFlexibleBox::~RenderFlexibleBox() noexcept = default;

  void RenderFlexibleBox::ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                        LayoutUnit &maxLogicalWidth) const noexcept
  {
    auto addScrollbarWidth = [&]
    {
      LayoutUnit scrollbarWidth(ScrollbarLogicalWidth());
      maxLogicalWidth += scrollbarWidth;
      minLogicalWidth += scrollbarWidth;
    };

    if (ShouldApplySizeOrInlineSizeContainment())
    {
      if (auto width = ExplicitIntrinsicInnerLogicalWidth())
      {
        minLogicalWidth = *width;
        maxLogicalWidth = *width;
      }
      addScrollbarWidth();
      return;
    }

    LayoutUnit flexItemMinWidth;
    LayoutUnit flexItemMaxWidth;
    bool hadExcludedChildren = ComputePreferredWidthsForExcludedChildren(flexItemMinWidth, flexItemMaxWidth);

    // FIXME: We're ignoring flex-basis here and we shouldn't. We can't start
    // honoring it though until the flex shorthand stops setting it to 0. See
    // https://bugs.webkit.org/show_bug.cgi?id=116117 and
    // https://crbug.com/240765.
    size_t numItemsWithNormalLayout = 0;
    for (auto *flexItem = FirstChildBox(); flexItem; flexItem = flexItem->NextSiblingBox())
    {
      if (flexItem->IsOutOfFlowPositioned() || flexItem->IsExcludedFromNormalLayout())
      {
        continue;
      }
      ++numItemsWithNormalLayout;

      // Pre-layout orthogonal children in order to get a valid value for the preferred width.
      if (GetWritingMode().IsOrthogonal(flexItem->GetWritingMode()))
      {
        flexItem->LayoutIfNeeded();
      }

      LayoutUnit margin = MarginIntrinsicLogicalWidthForChild(*flexItem);

      LayoutUnit minPreferredLogicalWidth;
      LayoutUnit maxPreferredLogicalWidth;
      ComputeChildPreferredLogicalWidths(*flexItem, minPreferredLogicalWidth, maxPreferredLogicalWidth);

      minPreferredLogicalWidth += margin;
      maxPreferredLogicalWidth += margin;

      if (!IsColumnFlow())
      {
        maxLogicalWidth += maxPreferredLogicalWidth;
        if (IsMultiline())
        {
          // For multiline, the min preferred width is if you put a break between
          // each item.
          minLogicalWidth = std::max(minLogicalWidth, minPreferredLogicalWidth);
        }
        else
        {
          minLogicalWidth += minPreferredLogicalWidth;
        }
      }
      else
      {
        minLogicalWidth = std::max(minPreferredLogicalWidth, minLogicalWidth);
        maxLogicalWidth = std::max(maxPreferredLogicalWidth, maxLogicalWidth);
      }
    }

    if (!IsColumnFlow() && numItemsWithNormalLayout > 1)
    {
      LayoutUnit inlineGapSize =
        static_cast<int32>(numItemsWithNormalLayout - 1) * ComputeGap(GapType::BetweenItems);
      maxLogicalWidth += inlineGapSize;
      if (!IsMultiline())
      {
        minLogicalWidth += inlineGapSize;
      }
    }

    maxLogicalWidth = std::max(minLogicalWidth, maxLogicalWidth);

    // Due to negative margins, it is possible that we calculated a negative
    // intrinsic width. Make sure that we never return a negative width.
    minLogicalWidth = std::max(0_lu, minLogicalWidth);
    maxLogicalWidth = std::max(0_lu, maxLogicalWidth);

    if (hadExcludedChildren)
    {
      minLogicalWidth = std::max(minLogicalWidth, flexItemMinWidth);
      maxLogicalWidth = std::max(maxLogicalWidth, flexItemMaxWidth);
    }

    addScrollbarWidth();
  }

  void
    RenderFlexibleBox::ComputeChildIntrinsicLogicalWidths(RenderBox &flexBoxChild,
                                                          LayoutUnit &minPreferredLogicalWidth,
                                                          LayoutUnit &maxPreferredLogicalWidth) const noexcept
  {
    // Children excluded from normal layout are handled here too (e.g. legend when fieldset is set to flex).
    krys_debug_assert(flexBoxChild.IsFlexItem()
                      || (flexBoxChild.Parent() == this && flexBoxChild.IsExcludedFromNormalLayout()));

    // If the item cross size should use the definite container cross size then set the overriding size now so
    // the intrinsic sizes are properly computed in the presence of aspect ratios. The only exception is when
    // we are both a flex item&container, because our parent might have already set our overriding size.
    SetForScope flexItemIntrinsicSizeComputation(_inFlexItemIntrinsicWidthComputation, true);
    if (FlexItemCrossSizeShouldUseContainerCrossSize(flexBoxChild) && !IsFlexItem())
    {
      auto axis = MainAxisIsFlexItemInlineAxis(flexBoxChild) ? OverridingSizesScope::Axis::Block
                                                             : OverridingSizesScope::Axis::Inline;
      OverridingSizesScope overridingSizeScope(
        flexBoxChild, axis, ComputeCrossSizeForFlexItemUsingContainerCrossSize(flexBoxChild));
      RenderBlock::ComputeChildIntrinsicLogicalWidths(flexBoxChild, minPreferredLogicalWidth,
                                                      maxPreferredLogicalWidth);
      return;
    }

    OverridingSizesScope cleanOverridingSizesScope(flexBoxChild, OverridingSizesScope::Axis::Both);
    RenderBlock::ComputeChildIntrinsicLogicalWidths(flexBoxChild, minPreferredLogicalWidth,
                                                    maxPreferredLogicalWidth);
  }

  Maybe<LayoutUnit> RenderFlexibleBox::FirstLineBaseline() const noexcept
  {
    if ((IsWritingModeRoot() && !IsFlexItem()) || !_numberOfFlexItemsOnFirstLine
        || ShouldApplyLayoutContainment())
    {
      return {};
    }

    auto *baselineFlexItem = FlexItemForFirstBaseline();
    if (!baselineFlexItem)
    {
      return {};
    }

    auto baseline = Maybe<LayoutUnit> {};
    if (!IsColumnFlow() && !MainAxisIsFlexItemInlineAxis(*baselineFlexItem))
    {
      baseline = CrossAxisExtentForFlexItem(*baselineFlexItem);
    }
    else if (IsColumnFlow() && MainAxisIsFlexItemInlineAxis(*baselineFlexItem))
    {
      baseline = MainAxisExtentForFlexItem(*baselineFlexItem);
    }
    else if (auto firstLineBaseline = baselineFlexItem->FirstLineBaseline())
    {
      baseline = firstLineBaseline;
    }
    else
    {
      // FIXME: We should pass |direction| into firstLineBoxBaseline and stop bailing out if we're a writing
      // mode root. This would also fix some cases where the flexbox is orthogonal to its container.
      auto direction = IsHorizontalWritingMode() ? LineDirection::Horizontal : LineDirection::Vertical;
      auto flexboxWritingMode = Style().GetWritingMode();
      auto dominantBaseline = BaselineAlignmentState::DominantBaseline(flexboxWritingMode);
      baseline = BaselineAlignmentState::SynthesizedBaseline(
        *baselineFlexItem, dominantBaseline, flexboxWritingMode, direction, BaselineSynthesisEdge::BorderBox);
    }
    // Subpixel inline layout is off: the item's top is snapped to an integer.
    return LayoutUnit(baselineFlexItem->LogicalTop().ToInt()) + *baseline;
  }

  Maybe<LayoutUnit> RenderFlexibleBox::LastLineBaseline() const noexcept
  {
    if (IsWritingModeRoot() || !_numberOfFlexItemsOnLastLine || ShouldApplyLayoutContainment())
    {
      return {};
    }

    auto *baselineFlexItem = FlexItemForLastBaseline();
    if (!baselineFlexItem)
    {
      return {};
    }

    auto baseline = Maybe<LayoutUnit> {};
    if (!IsColumnFlow() && !MainAxisIsFlexItemInlineAxis(*baselineFlexItem))
    {
      baseline = CrossAxisExtentForFlexItem(*baselineFlexItem);
    }
    else if (IsColumnFlow() && MainAxisIsFlexItemInlineAxis(*baselineFlexItem))
    {
      baseline = MainAxisExtentForFlexItem(*baselineFlexItem);
    }
    else if (auto lastLineBaseline = baselineFlexItem->LastLineBaseline())
    {
      baseline = lastLineBaseline;
    }
    else
    {
      // FIXME: We should pass |direction| into firstLineBoxBaseline and stop bailing out if we're a writing
      // mode root. This would also fix some cases where the flexbox is orthogonal to its container.
      auto direction = IsHorizontalWritingMode() ? LineDirection::Horizontal : LineDirection::Vertical;
      auto flexboxWritingMode = Style().GetWritingMode();
      auto dominantBaseline = BaselineAlignmentState::DominantBaseline(flexboxWritingMode);
      baseline = BaselineAlignmentState::SynthesizedBaseline(
        *baselineFlexItem, dominantBaseline, flexboxWritingMode, direction, BaselineSynthesisEdge::BorderBox);
    }
    // Subpixel inline layout is off: the item's top is snapped to an integer.
    return LayoutUnit(baselineFlexItem->LogicalTop().ToInt()) + *baseline;
  }

  void RenderFlexibleBox::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBlock::StyleDidChange(diff, oldStyle);
    if (!oldStyle || diff != style::DifferenceResult::Layout)
    {
      return;
    }

    auto oldStyleAlignItemsIsStretch =
      oldStyle->GetAlignItems().Resolve(SelfAlignmentNormalBehavior()).Position() == ItemPosition::Stretch;
    for (auto &flexItem : ChildrenOfType<RenderBox>(*this))
    {
      // Flex items that were previously stretching need to be relayed out so we
      // can compute new available cross axis space. This is only necessary for
      // stretching since other alignment values don't change the size of the
      // box.
      if (oldStyleAlignItemsIsStretch)
      {
        auto normalBehavior = SelfAlignmentNormalBehavior();

        auto previousAlignment = [&]
        {
          if (!flexItem.Style().GetAlignSelf().IsAuto())
          {
            return flexItem.Style().GetAlignSelf().Resolve(normalBehavior).Position();
          }
          return oldStyle->GetAlignItems().Resolve(normalBehavior).Position();
        }();
        auto newAlignment = [&]
        {
          if (!flexItem.Style().GetAlignSelf().IsAuto())
          {
            return flexItem.Style().GetAlignSelf().Resolve(normalBehavior).Position();
          }
          return Style().GetAlignItems().Resolve(normalBehavior).Position();
        }();

        if (previousAlignment == ItemPosition::Stretch && previousAlignment != newAlignment)
        {
          flexItem.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
        }
      }
    }
  }

  void RenderFlexibleBox::LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit) noexcept
  {
    krys_debug_assert(NeedsLayout());

    if (relayoutChildren == RelayoutChildren::No)
    {
      SetForScope simplifiedLayoutScope(_inSimplifiedLayout, true);
      if (SimplifiedLayout())
      {
        return;
      }
    }

    // FIXME: Repaint (LayoutRepainter), once there's painting.

    ResetLogicalHeightBeforeLayoutIfNeeded();
    _relaidOutFlexItems.clear();

    bool oldInLayout = _inLayout;
    _inLayout = true;

    if (!Style().GetMarginTrim().IsNone())
    {
      InitializeMarginTrimState();
    }

    if (RecomputeLogicalWidth())
    {
      relayoutChildren = RelayoutChildren::Yes;
    }

    LayoutUnit previousHeight = LogicalHeight();
    SetLogicalHeight(BorderAndPaddingLogicalHeight() + LayoutUnit(ScrollbarLogicalHeight()));
    {
      // There are no reflections (-webkit-box-reflect was dropped).
      LayoutStateMaintainer statePusher(*this, LocationOffset(),
                                        IsTransformed() || GetWritingMode().IsBlockFlipped());

      // FIXME: The fragments' widths (preparePaginationBeforeBlockLayout), once there's pagination.

      _numberOfFlexItemsOnFirstLine = {};
      _numberOfFlexItemsOnLastLine = {};
      _justifyContentStartOverflow = 0_lu;

      BeginUpdateScrollInfoAfterLayoutTransaction();

      PrepareOrderIteratorAndMargins();

      // Fieldsets need to find their legend and position it inside the border of the object.
      // The legend then gets skipped during normal layout. The same is true for ruby text.
      // It doesn't get included in the normal layout process but is instead skipped.
      LayoutExcludedChildren(relayoutChildren);

      // FIXME: Repaint the flex items that moved (appendFlexItemFrameRects,
      // repaintFlexItemsDuringLayoutIfMoved), once there's painting.

      PerformFlexLayout(relayoutChildren);

      {
        SetForScope scrollbarLayout(_inPostFlexUpdateScrollbarLayout, true);
        EndAndCommitUpdateScrollInfoAfterLayoutTransaction();
      }

      if (LogicalHeight() != previousHeight)
      {
        relayoutChildren = RelayoutChildren::Yes;
      }

      if (IsDocumentElementRenderer())
      {
        LayoutOutOfFlowBoxes(RelayoutChildren::Yes);
      }
      else
      {
        LayoutOutOfFlowBoxes(relayoutChildren);
      }

      // FIXME: css3/flexbox/repaint-rtl-column.html seems to repaint more overflow than it needs to.
      ComputeOverflow(LayoutOverflowLogicalBottom(*this));

      UpdateDescendantTransformsAfterLayout();
    }

    UpdateLayerTransform();

    // We have to reset this, because changes to our ancestors' style can affect
    // this value. Also, this needs to be before we call updateAfterLayout, as
    // that function may re-enter this one.
    ResetHasDefiniteHeight();

    _inLayout = oldInLayout;
  }

  void RenderFlexibleBox::RepositionLogicalHeightDependentFlexItems(FlexLineStates &lineStates,
                                                                    LayoutUnit gapBetweenLines) noexcept
  {
    LayoutUnit crossAxisStartEdge = lineStates.empty() ? 0_lu : lineStates[0].CrossAxisOffset;
    // If we have a single line flexbox, the line height is all the available space. For flex-direction: row,
    // this means we need to use the height, so we do this after calling updateLogicalHeight.
    if (!IsMultiline() && !lineStates.empty())
    {
      lineStates[0].CrossAxisExtent = CrossAxisContentExtent();
    }

    AlignFlexLines(lineStates, gapBetweenLines);

    AlignFlexItems(lineStates);

    if (IsWrapReverse())
    {
      FlipForWrapReverse(lineStates, crossAxisStartEdge);
    }

    // direction:rtl + flex-direction:column means the cross-axis direction is
    // flipped.
    FlipForRightToLeftColumn(lineStates);
  }

  bool RenderFlexibleBox::MainAxisIsFlexItemInlineAxis(const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow() == flexItem.IsHorizontalWritingMode();
  }

  bool RenderFlexibleBox::IsColumnFlow() const noexcept
  {
    return Style().IsColumnFlexDirection();
  }

  bool RenderFlexibleBox::IsColumnOrRowReverse() const noexcept
  {
    return Style().GetFlexDirection() == FlexDirection::ColumnReverse
           || Style().GetFlexDirection() == FlexDirection::RowReverse;
  }

  bool RenderFlexibleBox::IsWrapReverse() const noexcept
  {
    return Style().GetFlexWrap() == FlexWrap::Reverse;
  }

  bool RenderFlexibleBox::IsHorizontalFlow() const noexcept
  {
    if (IsHorizontalWritingMode())
    {
      return !IsColumnFlow();
    }
    return IsColumnFlow();
  }

  bool RenderFlexibleBox::IsLeftToRightFlow() const noexcept
  {
    if (IsColumnFlow())
    {
      return GetWritingMode().BlockDirection() == FlowDirection::TopToBottom
             || GetWritingMode().BlockDirection() == FlowDirection::LeftToRight;
    }
    return GetWritingMode().IsLogicalLeftInlineStart()
           ^ (Style().GetFlexDirection() == FlexDirection::RowReverse);
  }

  RenderFlexibleBox::Direction RenderFlexibleBox::CrossAxisDirection() const noexcept
  {
    auto crossAxisDirection =
      Style().IsRowFlexDirection() ? GetWritingMode().BlockDirection() : GetWritingMode().InlineDirection();
    switch (crossAxisDirection)
    {
      case FlowDirection::TopToBottom:
        return IsWrapReverse() ? Direction::BottomToTop : Direction::TopToBottom;
      case FlowDirection::BottomToTop:
        return IsWrapReverse() ? Direction::TopToBottom : Direction::BottomToTop;
      case FlowDirection::LeftToRight:
        return IsWrapReverse() ? Direction::RightToLeft : Direction::LeftToRight;
      case FlowDirection::RightToLeft:
        return IsWrapReverse() ? Direction::LeftToRight : Direction::RightToLeft;
    }
    krys_debug_assert(false);
    return Direction::TopToBottom;
  }

  bool RenderFlexibleBox::IsMultiline() const noexcept
  {
    return Style().GetFlexWrap() != FlexWrap::NoWrap;
  }

  // https://drafts.csswg.org/css-flexbox/#min-size-auto
  bool RenderFlexibleBox::ShouldApplyMinSizeAutoForFlexItem(const RenderBox &flexItem) const noexcept
  {
    const auto &minSize = MinMainSizeLengthForFlexItem(flexItem);
    // min, max and fit-content are equivalent to the automatic size for block sizes
    // https://drafts.csswg.org/css-sizing-3/#valdef-width-min-content.
    bool flexItemBlockSizeIsEquivalentToAutomaticSize =
      !MainAxisIsFlexItemInlineAxis(flexItem)
      && (minSize.IsMinContent() || minSize.IsMaxContent() || minSize.IsFitContent());

    auto computedOverflowIsNotScrollable = [this, &flexItem]()
    {
      auto overflow = MainAxisOverflowForFlexItem(flexItem);
      return overflow == Overflow::Visible || overflow == Overflow::Clip;
    };

    return (minSize.IsAuto() || flexItemBlockSizeIsEquivalentToAutomaticSize)
           && computedOverflowIsNotScrollable();
  }

  bool RenderFlexibleBox::ShouldApplyMinBlockSizeAutoForFlexItem(const RenderBox &flexItem) const noexcept
  {
    return !MainAxisIsFlexItemInlineAxis(flexItem) && ShouldApplyMinSizeAutoForFlexItem(flexItem);
  }

  style::FlexBasis RenderFlexibleBox::FlexBasisForFlexItem(const RenderBox &flexItem) const noexcept
  {
    auto flexBasis = flexItem.Style().GetFlexBasis();
    if (flexBasis.IsAuto())
    {
      flexBasis = PreferredMainSizeLengthForFlexItem(flexItem).AsFlexBasis();
    }
    return flexBasis;
  }

  LayoutUnit RenderFlexibleBox::CrossAxisExtentForFlexItem(const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow() ? flexItem.Height() : flexItem.Width();
  }

  LayoutUnit
    RenderFlexibleBox::CachedFlexItemIntrinsicContentLogicalHeight(const RenderBox &flexItem) const noexcept
  {
    if (auto *renderReplaced = DynamicDowncast<RenderReplaced>(flexItem))
    {
      return renderReplaced->IntrinsicLogicalHeight();
    }

    if (auto entry = _intrinsicContentLogicalHeights.find(&flexItem);
        entry != _intrinsicContentLogicalHeights.end())
    {
      return entry->second;
    }

    return flexItem.ContentBoxLogicalHeight();
  }

  void RenderFlexibleBox::SetCachedFlexItemIntrinsicContentLogicalHeight(const RenderBox &flexItem,
                                                                         LayoutUnit height) noexcept
  {
    if (flexItem.IsRenderReplaced())
    {
      return; // Replaced elements know their intrinsic height already, so save space by not caching.
    }
    _intrinsicContentLogicalHeights.insert_or_assign(&flexItem, height);
  }

  void RenderFlexibleBox::ClearCachedFlexItemIntrinsicContentLogicalHeight(const RenderBox &flexItem) noexcept
  {
    if (flexItem.IsRenderReplaced())
    {
      return; // Replaced elements know their intrinsic height already, so nothing to do.
    }
    _intrinsicContentLogicalHeights.erase(&flexItem);
  }

  LayoutUnit RenderFlexibleBox::FlexItemIntrinsicLogicalHeight(RenderBox &flexItem) const noexcept
  {
    // This should only be called if the logical height is the cross size
    krys_debug_assert(MainAxisIsFlexItemInlineAxis(flexItem));
    if (NeedToStretchFlexItemLogicalHeight(flexItem))
    {
      LayoutUnit flexItemContentHeight = CachedFlexItemIntrinsicContentLogicalHeight(flexItem);
      LayoutUnit flexItemLogicalHeight = flexItemContentHeight + LayoutUnit(flexItem.ScrollbarLogicalHeight())
                                         + flexItem.BorderAndPaddingLogicalHeight();
      return flexItem.ConstrainLogicalHeightByMinMax(flexItemLogicalHeight, flexItemContentHeight);
    }
    return flexItem.LogicalHeight();
  }

  LayoutUnit RenderFlexibleBox::FlexItemIntrinsicLogicalWidth(RenderBox &flexItem) noexcept
  {
    // This should only be called if the logical width is the cross size
    krys_debug_assert(!MainAxisIsFlexItemInlineAxis(flexItem));
    if (FlexItemCrossSizeIsDefinite(flexItem, flexItem.Style().LogicalWidth()))
    {
      return flexItem.LogicalWidth();
    }

    LogicalExtentComputedValues values;
    {
      OverridingSizesScope cleanOverridingWidthScope(flexItem, OverridingSizesScope::Axis::Inline);
      flexItem.ComputeLogicalWidth(values);
    }
    return values.Extent;
  }

  LayoutUnit RenderFlexibleBox::CrossAxisIntrinsicExtentForFlexItem(RenderBox &flexItem) noexcept
  {
    return MainAxisIsFlexItemInlineAxis(flexItem) ? FlexItemIntrinsicLogicalHeight(flexItem)
                                                  : FlexItemIntrinsicLogicalWidth(flexItem);
  }

  LayoutUnit RenderFlexibleBox::MainAxisExtentForFlexItem(const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow() ? flexItem.Size().Width() : flexItem.Size().Height();
  }

  LayoutUnit RenderFlexibleBox::MainAxisContentExtentForFlexItemIncludingScrollbar(
    const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow()
             ? flexItem.ContentBoxWidth() + LayoutUnit(flexItem.VerticalScrollbarWidth())
             : flexItem.ContentBoxHeight() + LayoutUnit(flexItem.HorizontalScrollbarHeight());
  }

  LayoutUnit RenderFlexibleBox::CrossAxisExtent() const noexcept
  {
    return IsHorizontalFlow() ? Size().Height() : Size().Width();
  }

  LayoutUnit RenderFlexibleBox::MainAxisExtent() const noexcept
  {
    return IsHorizontalFlow() ? Size().Width() : Size().Height();
  }

  LayoutUnit RenderFlexibleBox::CrossAxisContentExtent() const noexcept
  {
    return IsHorizontalFlow() ? ContentBoxHeight() : ContentBoxWidth();
  }

  LayoutUnit RenderFlexibleBox::MainAxisContentExtent(LayoutUnit contentLogicalHeight) noexcept
  {
    if (!IsColumnFlow())
    {
      return ContentBoxLogicalWidth();
    }

    LayoutUnit borderPaddingAndScrollbar =
      BorderAndPaddingLogicalHeight() + LayoutUnit(ScrollbarLogicalHeight());
    LayoutUnit borderBoxLogicalHeight = contentLogicalHeight + borderPaddingAndScrollbar;
    auto computedValues = ComputeLogicalHeight(borderBoxLogicalHeight, LogicalTop());
    if (computedValues.Extent == LayoutUnit::Max())
    {
      return computedValues.Extent;
    }
    return std::max(0_lu, computedValues.Extent - borderPaddingAndScrollbar);
  }

  template <typename TSize>
  Maybe<LayoutUnit> RenderFlexibleBox::ComputeMainAxisExtentForFlexItem(RenderBox &flexItem,
                                                                        const TSize &size) noexcept
  {
    // If we have a horizontal flow, that means the main size is the width.
    // That's the logical width for horizontal writing modes, and the logical
    // height in vertical writing modes. For a vertical flow, main size is the
    // height, so it's the inverse. So we need the logical width if we have a
    // horizontal flow and horizontal writing mode, or vertical flow and vertical
    // writing mode. Otherwise we need the logical height.
    if (!MainAxisIsFlexItemInlineAxis(flexItem))
    {
      // We don't have to check for "auto" here - computeContentLogicalHeight
      // will just return a null Optional for that case anyway. It's safe to access
      // scrollbarLogicalHeight here because ComputeNextFlexLine will have
      // already forced layout on the child. We previously did a layout out the child
      // if necessary (see ComputeNextFlexLine and the call to
      // flexItemHasIntrinsicMainAxisSize) so we can be sure that the two height
      // calls here will return up-to-date data.
      auto height =
        flexItem.ComputeContentLogicalHeight(size, CachedFlexItemIntrinsicContentLogicalHeight(flexItem));
      if (!height)
      {
        return height;
      }
      // Tables interpret overriding sizes as the size of captions + rows. However the specified height of a
      // table only includes the size of the rows. That's why we need to add the size of the captions here so
      // that the table layout algorithm behaves appropriately.
      LayoutUnit captionsHeight;
      if (auto *table = DynamicDowncast<RenderTable>(flexItem);
          table && FlexItemMainSizeIsDefinite(flexItem, size))
      {
        captionsHeight = table->SumCaptionsLogicalHeight();
      }
      return *height + LayoutUnit(flexItem.ScrollbarLogicalHeight()) + captionsHeight;
    }

    // computeLogicalWidth always re-computes the intrinsic widths. However, when
    // our logical width is auto, we can just use our cached value. So let's do
    // that here. (Compare code in RenderBlock::computePreferredLogicalWidths)
    if (flexItem.Style().LogicalWidth().IsAuto() && !FlexItemHasAspectRatio(flexItem))
    {
      if (size.IsMinContent())
      {
        if (flexItem.ShouldInvalidatePreferredWidths())
        {
          flexItem.SetNeedsPreferredWidthsUpdate(RenderObjectMarkingBehavior::MarkOnlyThis);
        }
        return flexItem.MinPreferredLogicalWidth() - flexItem.BorderAndPaddingLogicalWidth();
      }
      if (size.IsMaxContent())
      {
        if (flexItem.ShouldInvalidatePreferredWidths())
        {
          flexItem.SetNeedsPreferredWidthsUpdate(RenderObjectMarkingBehavior::MarkOnlyThis);
        }
        return flexItem.MaxPreferredLogicalWidth() - flexItem.BorderAndPaddingLogicalWidth();
      }
    }

    auto mainAxisWidth = IsColumnFlow()
                           ? AvailableLogicalHeight(AvailableLogicalHeightType::ExcludeMarginBorderPadding)
                           : ContentBoxLogicalWidth();
    return flexItem.ComputeLogicalWidthUsing(size, mainAxisWidth, *this)
           - flexItem.BorderAndPaddingLogicalWidth();
  }

  FlowDirection RenderFlexibleBox::TransformedBlockFlowDirection() const noexcept
  {
    if (!IsColumnFlow())
    {
      return GetWritingMode().BlockDirection();
    }
    return GetWritingMode().InlineDirection();
  }

  LayoutUnit RenderFlexibleBox::FlowAwareBorderStart() const noexcept
  {
    if (IsHorizontalFlow())
    {
      return IsLeftToRightFlow() ? BorderLeft() : BorderRight();
    }
    return IsLeftToRightFlow() ? BorderTop() : BorderBottom();
  }

  LayoutUnit RenderFlexibleBox::FlowAwareBorderEnd() const noexcept
  {
    if (IsHorizontalFlow())
    {
      return IsLeftToRightFlow() ? BorderRight() : BorderLeft();
    }
    return IsLeftToRightFlow() ? BorderBottom() : BorderTop();
  }

  LayoutUnit RenderFlexibleBox::FlowAwareBorderBefore() const noexcept
  {
    switch (TransformedBlockFlowDirection())
    {
      case FlowDirection::TopToBottom: return BorderTop();
      case FlowDirection::BottomToTop: return BorderBottom();
      case FlowDirection::LeftToRight: return BorderLeft();
      case FlowDirection::RightToLeft: return BorderRight();
    }
    krys_debug_assert(false);
    return BorderTop();
  }

  LayoutUnit RenderFlexibleBox::FlowAwareBorderAfter() const noexcept
  {
    switch (TransformedBlockFlowDirection())
    {
      case FlowDirection::TopToBottom: return BorderBottom();
      case FlowDirection::BottomToTop: return BorderTop();
      case FlowDirection::LeftToRight: return BorderRight();
      case FlowDirection::RightToLeft: return BorderLeft();
    }
    krys_debug_assert(false);
    return BorderTop();
  }

  LayoutUnit RenderFlexibleBox::FlowAwarePaddingStart() const noexcept
  {
    if (IsHorizontalFlow())
    {
      return IsLeftToRightFlow() ? PaddingLeft() : PaddingRight();
    }
    return IsLeftToRightFlow() ? PaddingTop() : PaddingBottom();
  }

  LayoutUnit RenderFlexibleBox::FlowAwarePaddingEnd() const noexcept
  {
    if (IsHorizontalFlow())
    {
      return IsLeftToRightFlow() ? PaddingRight() : PaddingLeft();
    }
    return IsLeftToRightFlow() ? PaddingBottom() : PaddingTop();
  }

  LayoutUnit RenderFlexibleBox::FlowAwarePaddingBefore() const noexcept
  {
    switch (TransformedBlockFlowDirection())
    {
      case FlowDirection::TopToBottom: return PaddingTop();
      case FlowDirection::BottomToTop: return PaddingBottom();
      case FlowDirection::LeftToRight: return PaddingLeft();
      case FlowDirection::RightToLeft: return PaddingRight();
    }
    krys_debug_assert(false);
    return PaddingTop();
  }

  LayoutUnit RenderFlexibleBox::FlowAwarePaddingAfter() const noexcept
  {
    switch (TransformedBlockFlowDirection())
    {
      case FlowDirection::TopToBottom: return PaddingBottom();
      case FlowDirection::BottomToTop: return PaddingTop();
      case FlowDirection::LeftToRight: return PaddingRight();
      case FlowDirection::RightToLeft: return PaddingLeft();
    }
    krys_debug_assert(false);
    return PaddingTop();
  }

  LayoutUnit RenderFlexibleBox::FlowAwareMarginStartForFlexItem(const RenderBox &flexItem) const noexcept
  {
    if (IsHorizontalFlow())
    {
      return IsLeftToRightFlow() ? flexItem.MarginLeft() : flexItem.MarginRight();
    }
    return IsLeftToRightFlow() ? flexItem.MarginTop() : flexItem.MarginBottom();
  }

  LayoutUnit RenderFlexibleBox::FlowAwareMarginEndForFlexItem(const RenderBox &flexItem) const noexcept
  {
    if (IsHorizontalFlow())
    {
      return IsLeftToRightFlow() ? flexItem.MarginRight() : flexItem.MarginLeft();
    }
    return IsLeftToRightFlow() ? flexItem.MarginBottom() : flexItem.MarginTop();
  }

  LayoutUnit RenderFlexibleBox::FlowAwareMarginBeforeForFlexItem(const RenderBox &flexItem) const noexcept
  {
    switch (TransformedBlockFlowDirection())
    {
      case FlowDirection::TopToBottom: return flexItem.MarginTop();
      case FlowDirection::BottomToTop: return flexItem.MarginBottom();
      case FlowDirection::LeftToRight: return flexItem.MarginLeft();
      case FlowDirection::RightToLeft: return flexItem.MarginRight();
    }
    krys_debug_assert(false);
    return MarginTop();
  }

  void RenderFlexibleBox::InitializeMarginTrimState() noexcept
  {
    // When computeIntrinsicLogicalWidth goes through each of the children, it
    // will include the margins when computing the flexbox's min and max widths.
    // We need to trim the margins of the first and last child early so that
    // these margins do not incorrectly constribute to the box's min/max width
    auto marginTrim = Style().GetMarginTrim();
    auto isRowsFlexbox = IsHorizontalFlow();
    if (auto *flexItem = FirstInFlowChildBox();
        flexItem && marginTrim.Contains(style::MarginTrimSide::InlineStart))
    {
      (isRowsFlexbox ? _marginTrimItems.ItemsAtFlexLineStart : _marginTrimItems.ItemsOnFirstFlexLine)
        .insert(flexItem);
    }
    if (auto *flexItem = LastInFlowChildBox();
        flexItem && marginTrim.Contains(style::MarginTrimSide::InlineEnd))
    {
      (isRowsFlexbox ? _marginTrimItems.ItemsAtFlexLineEnd : _marginTrimItems.ItemsOnLastFlexLine)
        .insert(flexItem);
    }
  }

  bool RenderFlexibleBox::CanFitItemWithTrimmedMarginEnd(const FlexLayoutItem &flexLayoutItem,
                                                         LayoutUnit sumHypotheticalMainSize,
                                                         LayoutUnit lineBreakLength) const noexcept
  {
    auto marginTrim = Style().GetMarginTrim();
    if ((IsHorizontalFlow() && marginTrim.Contains(style::MarginTrimSide::InlineEnd))
        || (IsColumnFlow() && marginTrim.Contains(style::MarginTrimSide::BlockEnd)))
    {
      return sumHypotheticalMainSize + flexLayoutItem.HypotheticalMainAxisMarginBoxSize()
               - FlowAwareMarginEndForFlexItem(*flexLayoutItem.Renderer)
             <= lineBreakLength;
    }
    return false;
  }

  void RenderFlexibleBox::RemoveMarginEndFromFlexSizes(FlexLayoutItem &flexLayoutItem,
                                                       LayoutUnit &sumFlexBaseSize,
                                                       LayoutUnit &sumHypotheticalMainSize) const noexcept
  {
    LayoutUnit margin;
    if (IsHorizontalFlow())
    {
      margin = flexLayoutItem.Renderer->MarginEnd(GetWritingMode());
    }
    else
    {
      margin = flexLayoutItem.Renderer->MarginAfter(GetWritingMode());
    }
    sumFlexBaseSize -= margin;
    sumHypotheticalMainSize -= margin;
  }

  LayoutUnit RenderFlexibleBox::MainAxisMarginExtentForFlexItem(const RenderBox &flexItem) const noexcept
  {
    if (!flexItem.NeedsLayout())
    {
      return IsHorizontalFlow() ? flexItem.HorizontalMarginExtent() : flexItem.VerticalMarginExtent();
    }

    LayoutUnit marginStart;
    LayoutUnit marginEnd;
    if (IsHorizontalFlow())
    {
      flexItem.ComputeInlineDirectionMargins(*this, flexItem.ContainingBlockLogicalWidthForContent(), null,
                                             flexItem.LogicalWidth(), marginStart, marginEnd);
    }
    else
    {
      flexItem.ComputeBlockDirectionMargins(*this, marginStart, marginEnd);
    }
    return marginStart + marginEnd;
  }

  LayoutUnit RenderFlexibleBox::CrossAxisMarginExtentForFlexItem(const RenderBox &flexItem) const noexcept
  {
    if (!flexItem.NeedsLayout())
    {
      return IsHorizontalFlow() ? flexItem.VerticalMarginExtent() : flexItem.HorizontalMarginExtent();
    }

    LayoutUnit marginStart;
    LayoutUnit marginEnd;
    if (IsHorizontalFlow())
    {
      flexItem.ComputeBlockDirectionMargins(*this, marginStart, marginEnd);
    }
    else
    {
      flexItem.ComputeInlineDirectionMargins(*this, flexItem.ContainingBlockLogicalWidthForContent(), null,
                                             flexItem.LogicalWidth(), marginStart, marginEnd);
    }
    return marginStart + marginEnd;
  }

  bool RenderFlexibleBox::IsChildEligibleForMarginTrim(style::MarginTrimSide marginTrimSide,
                                                       const RenderBox &flexItem) const noexcept
  {
    krys_debug_assert(Style().GetMarginTrim().Contains(marginTrimSide));
    auto isMarginParallelWithMainAxis = [this](style::MarginTrimSide marginTrimSide)
    {
      if (IsHorizontalFlow())
      {
        return marginTrimSide == style::MarginTrimSide::BlockStart
               || marginTrimSide == style::MarginTrimSide::BlockEnd;
      }
      return marginTrimSide == style::MarginTrimSide::InlineStart
             || marginTrimSide == style::MarginTrimSide::InlineEnd;
    };
    auto isStartSide = marginTrimSide == style::MarginTrimSide::BlockStart
                       || marginTrimSide == style::MarginTrimSide::InlineStart;
    if (isMarginParallelWithMainAxis(marginTrimSide))
    {
      return isStartSide ? _marginTrimItems.ItemsOnFirstFlexLine.contains(&flexItem)
                         : _marginTrimItems.ItemsOnLastFlexLine.contains(&flexItem);
    }
    return isStartSide ? _marginTrimItems.ItemsAtFlexLineStart.contains(&flexItem)
                       : _marginTrimItems.ItemsAtFlexLineEnd.contains(&flexItem);
  }

  bool RenderFlexibleBox::ShouldTrimMainAxisMarginStart() const noexcept
  {
    if (IsHorizontalFlow())
    {
      return Style().GetMarginTrim().Contains(style::MarginTrimSide::InlineStart);
    }
    return Style().GetMarginTrim().Contains(style::MarginTrimSide::BlockStart);
  }

  bool RenderFlexibleBox::ShouldTrimMainAxisMarginEnd() const noexcept
  {
    if (IsHorizontalFlow())
    {
      return Style().GetMarginTrim().Contains(style::MarginTrimSide::InlineEnd);
    }
    return Style().GetMarginTrim().Contains(style::MarginTrimSide::BlockEnd);
  }

  bool RenderFlexibleBox::ShouldTrimCrossAxisMarginStart() const noexcept
  {
    if (IsHorizontalFlow())
    {
      return Style().GetMarginTrim().Contains(style::MarginTrimSide::BlockStart);
    }
    return Style().GetMarginTrim().Contains(style::MarginTrimSide::InlineStart);
  }

  bool RenderFlexibleBox::ShouldTrimCrossAxisMarginEnd() const noexcept
  {
    if (IsHorizontalFlow())
    {
      return Style().GetMarginTrim().Contains(style::MarginTrimSide::BlockEnd);
    }
    return Style().GetMarginTrim().Contains(style::MarginTrimSide::InlineEnd);
  }

  void RenderFlexibleBox::TrimMainAxisMarginStart(const FlexLayoutItem &flexLayoutItem) noexcept
  {
    auto horizontalFlow = IsHorizontalFlow();
    flexLayoutItem.MainAxisMargin -= horizontalFlow ? flexLayoutItem.Renderer->MarginStart(GetWritingMode())
                                                    : flexLayoutItem.Renderer->MarginBefore(GetWritingMode());
    SetTrimmedMarginForChild(*flexLayoutItem.Renderer, horizontalFlow ? style::MarginTrimSide::InlineStart
                                                                      : style::MarginTrimSide::BlockStart);
    _marginTrimItems.ItemsAtFlexLineStart.insert(flexLayoutItem.Renderer);
  }

  void RenderFlexibleBox::TrimMainAxisMarginEnd(const FlexLayoutItem &flexLayoutItem) noexcept
  {
    auto horizontalFlow = IsHorizontalFlow();
    flexLayoutItem.MainAxisMargin -= horizontalFlow ? flexLayoutItem.Renderer->MarginEnd(GetWritingMode())
                                                    : flexLayoutItem.Renderer->MarginAfter(GetWritingMode());
    SetTrimmedMarginForChild(*flexLayoutItem.Renderer, horizontalFlow ? style::MarginTrimSide::InlineEnd
                                                                      : style::MarginTrimSide::BlockEnd);
    _marginTrimItems.ItemsAtFlexLineEnd.insert(flexLayoutItem.Renderer);
  }

  void RenderFlexibleBox::TrimCrossAxisMarginStart(const FlexLayoutItem &flexLayoutItem) noexcept
  {
    SetTrimmedMarginForChild(*flexLayoutItem.Renderer, IsHorizontalFlow()
                                                         ? style::MarginTrimSide::BlockStart
                                                         : style::MarginTrimSide::InlineStart);
    _marginTrimItems.ItemsOnFirstFlexLine.insert(flexLayoutItem.Renderer);
  }

  void RenderFlexibleBox::TrimCrossAxisMarginEnd(const FlexLayoutItem &flexLayoutItem) noexcept
  {
    SetTrimmedMarginForChild(*flexLayoutItem.Renderer, IsHorizontalFlow() ? style::MarginTrimSide::BlockEnd
                                                                          : style::MarginTrimSide::InlineEnd);
    _marginTrimItems.ItemsOnLastFlexLine.insert(flexLayoutItem.Renderer);
  }

  LayoutUnit RenderFlexibleBox::CrossAxisScrollbarExtent() const noexcept
  {
    return LayoutUnit(IsHorizontalFlow() ? HorizontalScrollbarHeight() : VerticalScrollbarWidth());
  }

  LayoutUnit RenderFlexibleBox::MainAxisScrollbarExtent() const noexcept
  {
    return LayoutUnit(IsHorizontalFlow() ? VerticalScrollbarWidth() : HorizontalScrollbarHeight());
  }

  LayoutPoint RenderFlexibleBox::FlowAwareLocationForFlexItem(const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow() ? flexItem.Location() : flexItem.Location().TransposedPoint();
  }

  const style::PreferredSize &
    RenderFlexibleBox::PreferredCrossSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow() ? flexItem.Style().Height() : flexItem.Style().Width();
  }

  const style::MinimumSize &
    RenderFlexibleBox::MinCrossSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow() ? flexItem.Style().MinHeight() : flexItem.Style().MinWidth();
  }

  const style::MaximumSize &
    RenderFlexibleBox::MaxCrossSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow() ? flexItem.Style().MaxHeight() : flexItem.Style().MaxWidth();
  }

  const style::PreferredSize &
    RenderFlexibleBox::PreferredMainSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow() ? flexItem.Style().Width() : flexItem.Style().Height();
  }

  const style::MinimumSize &
    RenderFlexibleBox::MinMainSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow() ? flexItem.Style().MinWidth() : flexItem.Style().MinHeight();
  }

  const style::MaximumSize &
    RenderFlexibleBox::MaxMainSizeLengthForFlexItem(const RenderBox &flexItem) const noexcept
  {
    return IsHorizontalFlow() ? flexItem.Style().MaxWidth() : flexItem.Style().MaxHeight();
  }

  double RenderFlexibleBox::PreferredAspectRatioForFlexItem(const RenderBox &flexItem) const noexcept
  {
    auto flexItemAspectRatio = [&]
    {
      auto flexItemIntrinsicSize =
        LayoutSize {flexItem.IntrinsicLogicalWidth(), flexItem.IntrinsicLogicalHeight()};
      // FIXME: SVG roots compute their intrinsic aspect ratio, once there's SVG.
      if (flexItem.Style().GetAspectRatio().IsRatio()
          || (flexItem.Style().GetAspectRatio().IsAutoAndRatio() && flexItemIntrinsicSize.IsEmpty()))
      {
        return flexItem.Style().LogicalAspectRatio();
      }
      if (auto *replacedElement = DynamicDowncast<RenderReplaced>(flexItem))
      {
        return replacedElement->ComputeIntrinsicAspectRatio();
      }

      krys_debug_assert(flexItem.IntrinsicLogicalHeight());
      return flexItem.IntrinsicLogicalWidth().ToDouble() / flexItem.IntrinsicLogicalHeight().ToDouble();
    };

    if (MainAxisIsFlexItemInlineAxis(flexItem))
    {
      return flexItemAspectRatio();
    }
    return 1 / flexItemAspectRatio();
  }

  template <typename TSize>
  LayoutUnit
    RenderFlexibleBox::ComputeMainSizeFromAspectRatioUsing(const RenderBox &flexItem,
                                                           const TSize &crossSizeLength) const noexcept
  {
    krys_debug_assert(FlexItemHasAspectRatio(flexItem));

    // `crossSize` is border-box size if box-sizing is border-box, and content-box otherwise.

    auto crossSizeOptional = crossSizeLength.SwitchOn(
      [&](const typename TSize::Fixed &fixedCrossSizeLength) -> Maybe<LayoutUnit>
      { return LayoutUnit(fixedCrossSizeLength.ResolveZoom(flexItem.Style().UsedZoomForLength())); },
      [&](const typename TSize::Percentage &percentageCrossSizeLength) -> Maybe<LayoutUnit>
      {
        return MainAxisIsFlexItemInlineAxis(flexItem)
                 ? flexItem.ComputePercentageLogicalHeight(TSize {percentageCrossSizeLength})
                 : AdjustBorderBoxLogicalWidthForBoxSizing(
                     style::Evaluate<LayoutUnit>(percentageCrossSizeLength, ContentBoxWidth()));
      },
      [&](const typename TSize::Calc &calcCrossSizeLength) -> Maybe<LayoutUnit>
      {
        return MainAxisIsFlexItemInlineAxis(flexItem)
                 ? flexItem.ComputePercentageLogicalHeight(TSize {typename TSize::Calc {calcCrossSizeLength}})
                 : AdjustBorderBoxLogicalWidthForBoxSizing(style::Evaluate<LayoutUnit>(
                     calcCrossSizeLength, ContentBoxWidth(), flexItem.Style().UsedZoomForLength()));
      },
      [&](const css::keywords::Auto &) -> Maybe<LayoutUnit>
      {
        krys_debug_assert(FlexItemCrossSizeShouldUseContainerCrossSize(flexItem));
        return ComputeCrossSizeForFlexItemUsingContainerCrossSize(flexItem);
      },
      [&](const auto &) -> Maybe<LayoutUnit>
      {
        krys_debug_assert(false);
        return {};
      });
    if (!crossSizeOptional)
    {
      return 0_lu;
    }

    auto crossSize = *crossSizeOptional;
    auto flexItemIntrinsicSize = flexItem.IntrinsicSize();
    LayoutUnit borderAndPadding;
    if (flexItem.Style().GetAspectRatio().IsRatio()
        || (flexItem.Style().GetAspectRatio().IsAutoAndRatio() && flexItemIntrinsicSize.IsEmpty()))
    {
      if (flexItem.Style().BoxSizingForAspectRatio() == BoxSizing::ContentBox)
      {
        crossSize -= IsHorizontalFlow() ? flexItem.VerticalBorderAndPaddingExtent()
                                        : flexItem.HorizontalBorderAndPaddingExtent();
      }
      else
      {
        borderAndPadding = IsHorizontalFlow() ? flexItem.HorizontalBorderAndPaddingExtent()
                                              : flexItem.VerticalBorderAndPaddingExtent();
      }
    }
    else
    {
      // We need to subtract the border and padding extent from the cross axis.
      // Furthermore, the sizing calculations that floor the content box size at zero when applying box-sizing
      // are also ignored. https://drafts.csswg.org/css-flexbox/#algo-main-item.
      if (flexItem.Style().GetBoxSizing() == BoxSizing::BorderBox)
      {
        crossSize -= IsHorizontalFlow() ? flexItem.VerticalBorderAndPaddingExtent()
                                        : flexItem.HorizontalBorderAndPaddingExtent();
      }
    }

    auto preferredAspectRatio = PreferredAspectRatioForFlexItem(flexItem);
    return std::max(0_lu, LayoutUnit(crossSize * preferredAspectRatio) - borderAndPadding);
  }

  void RenderFlexibleBox::SetFlowAwareLocationForFlexItem(RenderBox &flexItem,
                                                          const LayoutPoint &location) noexcept
  {
    if (IsHorizontalFlow())
    {
      flexItem.SetLocation(location);
    }
    else
    {
      flexItem.SetLocation(location.TransposedPoint());
    }
  }

  template <typename TSize>
  bool RenderFlexibleBox::CanComputePercentageFlexBasis(
    const RenderBox &flexItem, const TSize &flexBasis,
    UpdatePercentageHeightDescendants updateDescendants) noexcept
  {
    if (!IsColumnFlow() || _hasDefiniteHeight == SizeDefiniteness::Definite)
    {
      return true;
    }
    if (_hasDefiniteHeight == SizeDefiniteness::Indefinite)
    {
      return false;
    }

    auto isPercentResolveSuspended =
      View().FrameView().LayoutContext().IsPercentHeightResolveDisabledFor(flexItem);
    krys_debug_assert(!isPercentResolveSuspended || Is<RenderBlock>(flexItem));

    bool definite = !isPercentResolveSuspended
                    && flexItem.ComputePercentageLogicalHeight(flexBasis, updateDescendants).has_value();
    if (_inLayout && (IsHorizontalWritingMode() == flexItem.IsHorizontalWritingMode()))
    {
      // We can reach this code even while we're not laying ourselves out, such
      // as from mainSizeForPercentageResolution.
      _hasDefiniteHeight = definite ? SizeDefiniteness::Definite : SizeDefiniteness::Indefinite;
    }
    return definite;
  }

  template <typename TSize>
  bool RenderFlexibleBox::FlexItemMainSizeIsDefinite(const RenderBox &flexItem, const TSize &size) noexcept
  {
    if constexpr (!SameType<TSize, style::MaximumSize>)
    {
      if (size.IsAuto())
      {
        return false;
      }
    }
    if constexpr (SameType<TSize, style::FlexBasis>)
    {
      if (size.IsContent())
      {
        return false;
      }
    }
    if (!MainAxisIsFlexItemInlineAxis(flexItem) && (size.IsIntrinsic() || size.IsIntrinsicKeyword()))
    {
      return false;
    }
    if (size.IsPercentOrCalculated())
    {
      return CanComputePercentageFlexBasis(flexItem, size, UpdatePercentageHeightDescendants::No);
    }
    return true;
  }

  bool RenderFlexibleBox::FlexItemHasComputableAspectRatio(const RenderBox &flexItem) const noexcept
  {
    if (!FlexItemHasAspectRatio(flexItem))
    {
      return false;
    }
    return flexItem.IntrinsicSize().Height() || flexItem.Style().HasAspectRatio()
           || IsSVGRootWithIntrinsicAspectRatio(flexItem);
  }

  bool RenderFlexibleBox::FlexItemHasComputableAspectRatioAndCrossSizeIsConsideredDefinite(
    const RenderBox &flexItem) noexcept
  {
    return FlexItemHasComputableAspectRatio(flexItem)
           && (FlexItemCrossSizeIsDefinite(flexItem, PreferredCrossSizeLengthForFlexItem(flexItem))
               || FlexItemCrossSizeShouldUseContainerCrossSize(flexItem));
  }

  bool
    RenderFlexibleBox::FlexItemCrossSizeShouldUseContainerCrossSize(const RenderBox &flexItem) const noexcept
  {
    // 9.8 https://drafts.csswg.org/css-flexbox/#definite-sizes
    // 1. If a single-line flex container has a definite cross size, the automatic preferred outer cross size
    // of any stretched flex items is the flex container's inner cross size (clamped to the flex item's min
    // and max cross size) and is considered definite.
    if (!IsMultiline() && AlignmentForFlexItem(flexItem) == ItemPosition::Stretch
        && !HasAutoMarginsInCrossAxis(flexItem) && PreferredCrossSizeLengthForFlexItem(flexItem).IsAuto())
    {
      if (IsColumnFlow())
      {
        return true;
      }
      // This must be kept in sync with computeMainSizeFromAspectRatioUsing().
      auto &crossSize = IsHorizontalFlow() ? Style().Height() : Style().Width();
      return crossSize.IsFixed()
             || (crossSize.IsPercent() && AvailableLogicalHeightForPercentageComputation());
    }
    return false;
  }

  template <typename TSize>
  bool RenderFlexibleBox::FlexItemCrossSizeIsDefinite(const RenderBox &flexItem, const TSize &size) noexcept
  {
    if constexpr (!SameType<TSize, style::MaximumSize>)
    {
      if (size.IsAuto())
      {
        return false;
      }
    }

    if (size.IsPercentOrCalculated())
    {
      if (!MainAxisIsFlexItemInlineAxis(flexItem) || _hasDefiniteHeight == SizeDefiniteness::Definite)
      {
        return true;
      }
      if (_hasDefiniteHeight == SizeDefiniteness::Indefinite)
      {
        return false;
      }
      bool definite = static_cast<bool>(flexItem.ComputePercentageLogicalHeight(size));
      _hasDefiniteHeight = definite ? SizeDefiniteness::Definite : SizeDefiniteness::Indefinite;
      return definite;
    }
    // FIXME: Eventually we should support other types of sizes here.
    // Requires updating computeMainSizeFromAspectRatioUsing.
    return size.IsFixed();
  }

  void RenderFlexibleBox::CacheFlexItemMainSize(const RenderBox &flexItem) noexcept
  {
    krys_debug_assert(!flexItem.NeedsLayout());
    krys_debug_assert(!MainAxisIsFlexItemInlineAxis(flexItem));

    auto mainSize = [&]
    {
      auto flexBasis = FlexBasisForFlexItem(flexItem);
      if (flexBasis.IsPercentOrCalculated() && !FlexItemMainSizeIsDefinite(flexItem, flexBasis))
      {
        return CachedFlexItemIntrinsicContentLogicalHeight(flexItem)
               + flexItem.BorderAndPaddingLogicalHeight() + LayoutUnit(flexItem.ScrollbarLogicalHeight());
      }
      return flexItem.LogicalHeight();
    };

    _intrinsicSizeAlongMainAxis.insert_or_assign(&flexItem, mainSize());
    _relaidOutFlexItems.insert(&flexItem);
  }

  void RenderFlexibleBox::ClearCachedMainSizeForFlexItem(const RenderBox &flexItem) noexcept
  {
    _intrinsicSizeAlongMainAxis.erase(&flexItem);
  }

  // https://drafts.csswg.org/css-flexbox/#algo-main-item
  LayoutUnit RenderFlexibleBox::ComputeFlexBaseSizeForFlexItem(RenderBox &flexItem,
                                                               LayoutUnit mainAxisBorderAndPadding,
                                                               RelayoutChildren relayoutChildren) noexcept
  {
    auto flexBasis = FlexBasisForFlexItem(flexItem);
    ScopedFlexBasisAsFlexItemMainSize scoped(
      flexItem, flexBasis.TryPreferredSize().value_or(style::PreferredSize {css::keywords::MaxContent {}}),
      MainAxisIsFlexItemInlineAxis(flexItem));
    // FIXME: While we are supposed to ignore min/max here, clients of maybeCacheFlexItemMainIntrinsicSize may
    // expect min/max constrained size.
    SetForScope computingBaseSizesScope(_isComputingFlexBaseSizes, true);

    MaybeCacheFlexItemMainIntrinsicSize(flexItem, relayoutChildren);

    // 9.2.3 A.
    if (FlexItemMainSizeIsDefinite(flexItem, flexBasis))
    {
      return std::max(0_lu, ComputeMainAxisExtentForFlexItem(flexItem, flexBasis).value());
    }

    // 9.2.3 B.
    if (FlexItemHasComputableAspectRatioAndCrossSizeIsConsideredDefinite(flexItem))
    {
      auto &crossSizeLength = PreferredCrossSizeLengthForFlexItem(flexItem);
      return AdjustFlexItemSizeForAspectRatioCrossAxisMinAndMax(
        flexItem, ComputeMainSizeFromAspectRatioUsing(flexItem, crossSizeLength));
    }

    // FIXME: 9.2.3 C.
    // FIXME: 9.2.3 D.

    // 9.2.3 E.
    LayoutUnit mainAxisExtent;
    if (!MainAxisIsFlexItemInlineAxis(flexItem))
    {
      krys_debug_assert(!flexItem.NeedsLayout());
      krys_debug_assert(_intrinsicSizeAlongMainAxis.contains(&flexItem));
      mainAxisExtent = _intrinsicSizeAlongMainAxis[&flexItem];
    }
    else
    {
      // We don't need to add scrollbarLogicalWidth here because the preferred
      // width includes the scrollbar, even for overflow: auto.
      mainAxisExtent = flexItem.MaxPreferredLogicalWidth();
    }
    return mainAxisExtent - mainAxisBorderAndPadding;
  }

  void RenderFlexibleBox::PerformFlexLayout(RelayoutChildren relayoutChildren) noexcept
  {
    if (LayoutUsingFlexFormattingContext())
    {
      return;
    }

    // Set up our master list of flex items. All of the rest of the algorithm
    // should work off this list of a subset.
    // FIXME: That second part is not yet true.
    FlexLayoutItems allItems;
    for (auto *flexItem = _orderIterator.First(); flexItem; flexItem = _orderIterator.Next())
    {
      if (_orderIterator.ShouldSkipChild(*flexItem))
      {
        // Out-of-flow children are not flex items, so we skip them here.
        if (flexItem->IsOutOfFlowPositioned())
        {
          PrepareFlexItemForPositionedLayout(*flexItem);
        }
        continue;
      }
      allItems.push_back(ConstructFlexLayoutItem(*flexItem, relayoutChildren));
      // constructFlexItem() might set the override containing block height so any value cached for
      // definiteness might be incorrect.
      ResetHasDefiniteHeight();
    }

    if (allItems.empty())
    {
      if (HasLineIfEmpty())
      {
        auto minHeight =
          BorderAndPaddingLogicalHeight() + LineHeight() + LayoutUnit(ScrollbarLogicalHeight());
        if (Height() < minHeight)
        {
          SetLogicalHeight(minHeight);
        }
      }
      UpdateLogicalHeight();
      return;
    }

    FlexLineStates lineStates;
    const LayoutUnit lineBreakLength = MainAxisContentExtent(LayoutUnit::Max());
    LayoutUnit gapBetweenItems = ComputeGap(GapType::BetweenItems);
    LayoutUnit gapBetweenLines = ComputeGap(GapType::BetweenLines);
    LayoutUnit crossAxisOffset = FlowAwareBorderBefore() + FlowAwarePaddingBefore();
    size_t nextIndex = 0;
    size_t numLines = 0;
    while (auto lineData = ComputeNextFlexLine(nextIndex, allItems, lineBreakLength, gapBetweenItems))
    {
      ++numLines;

      auto &lineItems = lineData->LineItems;

      // Cross axis margins should only be trimmed if they are on the first/last flex line
      auto shouldTrimCrossAxisStart = ShouldTrimCrossAxisMarginStart() && lineStates.empty();
      auto shouldTrimCrossAxisEnd =
        ShouldTrimCrossAxisMarginEnd() && allItems.back().Renderer == lineItems.back().Renderer;
      if (shouldTrimCrossAxisStart || shouldTrimCrossAxisEnd)
      {
        for (auto &flexLayoutItem : lineItems)
        {
          if (shouldTrimCrossAxisStart)
          {
            TrimCrossAxisMarginStart(flexLayoutItem);
          }
          if (shouldTrimCrossAxisEnd)
          {
            TrimCrossAxisMarginEnd(flexLayoutItem);
          }
        }
      }
      auto containerMainInnerSize = MainAxisContentExtent(lineData->SumHypotheticalMainSize);
      // availableFreeSpace is the initial amount of free space in this flexbox.
      // remainingFreeSpace starts out at the same value but as we place and lay
      // out flex items we subtract from it. Note that both values can be
      // negative.
      auto remainingFreeSpace = containerMainInnerSize - lineData->SumFlexBaseSize;
      auto flexSign = (lineData->SumHypotheticalMainSize < containerMainInnerSize)
                        ? FlexSign::PositiveFlexibility
                        : FlexSign::NegativeFlexibility;
      FreezeInflexibleItems(flexSign, lineItems, remainingFreeSpace, lineData->TotalFlexGrow,
                            lineData->TotalFlexShrink, lineData->TotalWeightedFlexShrink);
      // The initial free space gets calculated after freezing inflexible items.
      // https://drafts.csswg.org/css-flexbox/#resolve-flexible-lengths step 3
      const LayoutUnit initialFreeSpace = remainingFreeSpace;
      while (!ResolveFlexibleLengths(flexSign, lineItems, initialFreeSpace, remainingFreeSpace,
                                     lineData->TotalFlexGrow, lineData->TotalFlexShrink,
                                     lineData->TotalWeightedFlexShrink))
      {
        krys_debug_assert(lineData->TotalFlexGrow >= 0);
        krys_debug_assert(lineData->TotalWeightedFlexShrink >= 0);
      }

      // Recalculate the remaining free space. The adjustment for flex factors
      // between 0..1 means we can't just use remainingFreeSpace here.
      remainingFreeSpace = containerMainInnerSize;
      for (auto &flexLayoutItem : lineItems)
      {
        krys_debug_assert(!flexLayoutItem.Renderer->IsOutOfFlowPositioned());
        remainingFreeSpace -= flexLayoutItem.FlexedMarginBoxSize();
      }
      remainingFreeSpace -= static_cast<int32>(lineItems.size() - 1) * gapBetweenItems;

      // This will move lineItems into a newly-created LineState.
      LayoutAndPlaceFlexItems(crossAxisOffset, lineItems, remainingFreeSpace, relayoutChildren, lineStates,
                              gapBetweenItems);
    }

    if (!lineStates.empty())
    {
      auto isWrapReverse = IsWrapReverse();
      auto firstLineItemsCountInOriginalOrder = lineStates.front().FlexLayoutItems.size();
      auto lastLineItemsCountInOriginalOrder = lineStates.back().FlexLayoutItems.size();

      _numberOfFlexItemsOnFirstLine =
        !isWrapReverse ? firstLineItemsCountInOriginalOrder : lastLineItemsCountInOriginalOrder;
      _numberOfFlexItemsOnLastLine =
        !isWrapReverse ? lastLineItemsCountInOriginalOrder : firstLineItemsCountInOriginalOrder;
    }

    if (HasLineIfEmpty())
    {
      // Even if computeNextFlexLine returns true, the flexbox might not have
      // a line because all our children might be out of flow positioned.
      // Instead of just checking if we have a line, make sure the flexbox
      // has at least a line's worth of height to cover this case.
      LayoutUnit minHeight =
        BorderAndPaddingLogicalHeight() + LineHeight() + LayoutUnit(ScrollbarLogicalHeight());
      if (Size().Height() < minHeight)
      {
        SetLogicalHeight(minHeight);
      }
    }

    if (!IsColumnFlow() && numLines > 1)
    {
      SetLogicalHeight(LogicalHeight()
                       + ComputeGap(GapType::BetweenLines) * static_cast<int32>(numLines - 1));
    }

    UpdateLogicalHeight();
    RepositionLogicalHeightDependentFlexItems(lineStates, gapBetweenLines);
  }

  Maybe<RenderFlexibleBox::FlexingLineData>
    RenderFlexibleBox::ComputeNextFlexLine(size_t &nextIndex, const FlexLayoutItems &allItems,
                                           LayoutUnit lineBreakLength, LayoutUnit gapBetweenItems) noexcept
  {
    if (nextIndex >= allItems.size())
    {
      return {};
    }

    FlexingLineData lineData;
    // Trim main axis margin for item at the start of the flex line
    if (nextIndex < allItems.size() && ShouldTrimMainAxisMarginStart())
    {
      TrimMainAxisMarginStart(allItems[nextIndex]);
    }
    for (; nextIndex < allItems.size(); ++nextIndex)
    {
      const auto &flexLayoutItem = allItems[nextIndex];
      auto &style = flexLayoutItem.Style();
      krys_debug_assert(!flexLayoutItem.Renderer->IsOutOfFlowPositioned());
      if (IsMultiline()
          && (lineData.SumHypotheticalMainSize + flexLayoutItem.HypotheticalMainAxisMarginBoxSize()
                > lineBreakLength
              && !CanFitItemWithTrimmedMarginEnd(flexLayoutItem, lineData.SumHypotheticalMainSize,
                                                 lineBreakLength))
          && !lineData.LineItems.empty())
      {
        break;
      }
      lineData.LineItems.push_back(flexLayoutItem);
      lineData.SumFlexBaseSize += flexLayoutItem.FlexBaseMarginBoxSize() + gapBetweenItems;
      lineData.TotalFlexGrow += style.GetFlexGrow().Value;
      lineData.TotalFlexShrink += style.GetFlexShrink().Value;
      lineData.TotalWeightedFlexShrink +=
        style.GetFlexShrink().Value * flexLayoutItem.FlexBaseContentSize.ToDouble();
      lineData.SumHypotheticalMainSize +=
        flexLayoutItem.HypotheticalMainAxisMarginBoxSize() + gapBetweenItems;
    }

    if (!lineData.LineItems.empty())
    {
      // We added a gap after every item but there shouldn't be one after the last item, so subtract it here.
      // Note that sums might be negative here due to negative margins in flex items.
      lineData.SumHypotheticalMainSize -= gapBetweenItems;
      lineData.SumFlexBaseSize -= gapBetweenItems;
    }

    krys_debug_assert(!lineData.LineItems.empty() || nextIndex == allItems.size());
    // Trim main axis margin for item at the end of the flex line
    if (!lineData.LineItems.empty() && ShouldTrimMainAxisMarginEnd())
    {
      auto &lastItem = lineData.LineItems.back();
      RemoveMarginEndFromFlexSizes(lastItem, lineData.SumFlexBaseSize, lineData.SumHypotheticalMainSize);
      TrimMainAxisMarginEnd(lastItem);
    }
    return lineData;
  }

  LayoutUnit RenderFlexibleBox::AutoMarginOffsetInMainAxis(const FlexLayoutItems &flexLayoutItems,
                                                           LayoutUnit &availableFreeSpace) noexcept
  {
    if (availableFreeSpace <= 0_lu)
    {
      return 0_lu;
    }

    int32 numberOfAutoMargins = 0;
    bool isHorizontal = IsHorizontalFlow();
    for (auto &flexLayoutItem : flexLayoutItems)
    {
      auto &flexItemStyle = flexLayoutItem.Style();
      krys_debug_assert(!flexLayoutItem.Renderer->IsOutOfFlowPositioned());
      if (isHorizontal)
      {
        if (flexItemStyle.MarginLeft().IsAuto())
        {
          ++numberOfAutoMargins;
        }
        if (flexItemStyle.MarginRight().IsAuto())
        {
          ++numberOfAutoMargins;
        }
      }
      else
      {
        if (flexItemStyle.MarginTop().IsAuto())
        {
          ++numberOfAutoMargins;
        }
        if (flexItemStyle.MarginBottom().IsAuto())
        {
          ++numberOfAutoMargins;
        }
      }
    }
    if (!numberOfAutoMargins)
    {
      return 0_lu;
    }

    LayoutUnit sizeOfAutoMargin = availableFreeSpace / numberOfAutoMargins;
    availableFreeSpace = 0_lu;
    return sizeOfAutoMargin;
  }

  void RenderFlexibleBox::UpdateAutoMarginsInMainAxis(RenderBox &flexItem,
                                                      LayoutUnit autoMarginOffset) noexcept
  {
    krys_debug_assert(autoMarginOffset >= 0_lu);

    if (IsHorizontalFlow())
    {
      if (flexItem.Style().MarginLeft().IsAuto())
      {
        flexItem.SetMarginLeft(autoMarginOffset);
      }
      if (flexItem.Style().MarginRight().IsAuto())
      {
        flexItem.SetMarginRight(autoMarginOffset);
      }
    }
    else
    {
      if (flexItem.Style().MarginTop().IsAuto())
      {
        flexItem.SetMarginTop(autoMarginOffset);
      }
      if (flexItem.Style().MarginBottom().IsAuto())
      {
        flexItem.SetMarginBottom(autoMarginOffset);
      }
    }
  }

  bool RenderFlexibleBox::HasAutoMarginsInCrossAxis(const RenderBox &flexItem) const noexcept
  {
    if (IsHorizontalFlow())
    {
      return flexItem.Style().MarginTop().IsAuto() || flexItem.Style().MarginBottom().IsAuto();
    }
    return flexItem.Style().MarginLeft().IsAuto() || flexItem.Style().MarginRight().IsAuto();
  }

  LayoutUnit RenderFlexibleBox::AvailableAlignmentSpaceForFlexItem(LayoutUnit lineCrossAxisExtent,
                                                                   const RenderBox &flexItem) noexcept
  {
    LayoutUnit flexItemCrossExtent =
      CrossAxisMarginExtentForFlexItem(flexItem) + CrossAxisExtentForFlexItem(flexItem);
    return lineCrossAxisExtent - flexItemCrossExtent;
  }

  bool RenderFlexibleBox::UpdateAutoMarginsInCrossAxis(RenderBox &flexItem,
                                                       LayoutUnit availableAlignmentSpace) noexcept
  {
    krys_debug_assert(!flexItem.IsOutOfFlowPositioned());
    krys_debug_assert(availableAlignmentSpace >= 0_lu);

    bool isHorizontal = IsHorizontalFlow();
    auto &topOrLeft = isHorizontal ? flexItem.Style().MarginTop() : flexItem.Style().MarginLeft();
    auto &bottomOrRight = isHorizontal ? flexItem.Style().MarginBottom() : flexItem.Style().MarginRight();
    if (topOrLeft.IsAuto() && bottomOrRight.IsAuto())
    {
      AdjustAlignmentForFlexItem(flexItem, availableAlignmentSpace / 2);
      if (isHorizontal)
      {
        flexItem.SetMarginTop(availableAlignmentSpace / 2);
        flexItem.SetMarginBottom(availableAlignmentSpace / 2);
      }
      else
      {
        flexItem.SetMarginLeft(availableAlignmentSpace / 2);
        flexItem.SetMarginRight(availableAlignmentSpace / 2);
      }
      return true;
    }
    bool shouldAdjustTopOrLeft = true;
    if (IsColumnFlow() && flexItem.GetWritingMode().IsInlineFlipped())
    {
      // For column flows, only make this adjustment if topOrLeft corresponds to
      // the "before" margin, so that flipForRightToLeftColumn will do the right
      // thing.
      shouldAdjustTopOrLeft = false;
    }
    if (!IsColumnFlow() && flexItem.GetWritingMode().IsBlockFlipped())
    {
      // If we are a flipped writing mode, we need to adjust the opposite side.
      // This is only needed for row flows because this only affects the
      // block-direction axis.
      shouldAdjustTopOrLeft = false;
    }

    if (topOrLeft.IsAuto())
    {
      if (shouldAdjustTopOrLeft)
      {
        AdjustAlignmentForFlexItem(flexItem, availableAlignmentSpace);
      }

      if (isHorizontal)
      {
        flexItem.SetMarginTop(availableAlignmentSpace);
      }
      else
      {
        flexItem.SetMarginLeft(availableAlignmentSpace);
      }
      return true;
    }

    if (bottomOrRight.IsAuto())
    {
      if (!shouldAdjustTopOrLeft)
      {
        AdjustAlignmentForFlexItem(flexItem, availableAlignmentSpace);
      }

      if (isHorizontal)
      {
        flexItem.SetMarginBottom(availableAlignmentSpace);
      }
      else
      {
        flexItem.SetMarginRight(availableAlignmentSpace);
      }
      return true;
    }
    return false;
  }

  LayoutUnit RenderFlexibleBox::MarginBoxAscentForFlexItem(const RenderBox &flexItem) noexcept
  {
    auto isHorizontalFlow = IsHorizontalFlow();
    auto direction = isHorizontalFlow ? LineDirection::Horizontal : LineDirection::Vertical;

    if (!MainAxisIsFlexItemInlineAxis(flexItem))
    {
      auto flexboxWritingMode = Style().GetWritingMode();
      auto alignmentContextAxis =
        Style().IsRowFlexDirection() ? LogicalBoxAxis::Inline : LogicalBoxAxis::Block;
      auto writingModeForSynthesis = BaselineAlignmentState::UsedWritingModeForBaselineAlignment(
        alignmentContextAxis, flexboxWritingMode, flexItem.GetWritingMode());
      return BaselineAlignmentState::SynthesizedBaseline(
               flexItem, BaselineAlignmentState::DominantBaseline(flexboxWritingMode),
               writingModeForSynthesis, direction, BaselineSynthesisEdge::BorderBox)
             + FlowAwareMarginBeforeForFlexItem(flexItem);
    }
    auto ascent = AlignmentForFlexItem(flexItem) == ItemPosition::LastBaseline ? flexItem.LastLineBaseline()
                                                                               : flexItem.FirstLineBaseline();
    if (!ascent)
    {
      auto flexboxWritingMode = Style().GetWritingMode();
      return BaselineAlignmentState::SynthesizedBaseline(
               flexItem, BaselineAlignmentState::DominantBaseline(flexboxWritingMode), flexboxWritingMode,
               direction, BaselineSynthesisEdge::BorderBox)
             + FlowAwareMarginBeforeForFlexItem(flexItem);
    }

    if (!flexItem.GetWritingMode().IsBlockMatchingAny(GetWritingMode()))
    {
      // Baseline from flex item with opposite block direction needs to be resolved as if flex item had the
      // same block direction.
      //  _____________________________ <- flex box top/left (e.g. writing-mode: vertical-rl)
      // |        __________________   |
      // |       |  20px |    80px  |<-- flex item with vertical-lr (top is at visual left)
      // |       |<----->|<-------->|  |
      // |       top     baseline   |  |
      // where computed baseline is 20px and resolved (as if flex item shares the block direction with flex
      // box) is 80px.
      ascent = flexItem.LogicalHeight() - *ascent;
    }

    if (isHorizontalFlow ? flexItem.IsScrollContainerY() : flexItem.IsScrollContainerX())
    {
      return std::max(0_lu, std::min(*ascent, CrossAxisExtentForFlexItem(flexItem)))
             + FlowAwareMarginBeforeForFlexItem(flexItem);
    }
    return *ascent + FlowAwareMarginBeforeForFlexItem(flexItem);
  }

  LayoutUnit RenderFlexibleBox::ComputeFlexItemMarginValue(const style::MarginEdge &margin) noexcept
  {
    // When resolving the margins, we use the content size for resolving percent and calc (for percents in
    // calc expressions) margins. Fortunately, percent margins are always computed with respect to the block's
    // width, even for margin-top and margin-bottom.
    return style::EvaluateMinimum<LayoutUnit>(margin, ContentBoxLogicalWidth(), Style().UsedZoomForLength());
  }

  void RenderFlexibleBox::PrepareOrderIteratorAndMargins() noexcept
  {
    OrderIteratorPopulator populator(_orderIterator);

    for (auto &flexItem : ChildrenOfType<RenderBox>(*this))
    {
      if (!populator.CollectChild(flexItem))
      {
        continue;
      }

      // Before running the flex algorithm, 'auto' has a margin of 0.
      // Also, if we're not auto sizing, we don't do a layout that computes the start/end margins.
      if (IsHorizontalFlow())
      {
        flexItem.SetMarginLeft(ComputeFlexItemMarginValue(flexItem.Style().MarginLeft()));
        flexItem.SetMarginRight(ComputeFlexItemMarginValue(flexItem.Style().MarginRight()));
      }
      else
      {
        flexItem.SetMarginTop(ComputeFlexItemMarginValue(flexItem.Style().MarginTop()));
        flexItem.SetMarginBottom(ComputeFlexItemMarginValue(flexItem.Style().MarginBottom()));
      }
    }
  }

  std::pair<LayoutUnit, LayoutUnit>
    RenderFlexibleBox::ComputeFlexItemMinMaxSizes(RenderBox &flexItem) noexcept
  {
    auto max = MaxMainSizeLengthForFlexItem(flexItem);
    Maybe<LayoutUnit> maxExtent = null;
    if (max.IsSpecified() || max.IsIntrinsic())
    {
      maxExtent = ComputeMainAxisExtentForFlexItem(flexItem, max);
    }

    auto min = MinMainSizeLengthForFlexItem(flexItem);
    // Intrinsic sizes in child's block axis are handled by the min-size:auto code path.
    if (min.IsSpecified() || (min.IsIntrinsic() && MainAxisIsFlexItemInlineAxis(flexItem)))
    {
      auto minExtent = ComputeMainAxisExtentForFlexItem(flexItem, min).value_or(0_lu);
      // We must never return a min size smaller than the min preferred size for tables.
      if (flexItem.IsRenderTable() && MainAxisIsFlexItemInlineAxis(flexItem))
      {
        minExtent = std::max(minExtent, flexItem.MinPreferredLogicalWidth());
      }
      return {minExtent, maxExtent.value_or(LayoutUnit::Max())};
    }

    if (ShouldApplyMinSizeAutoForFlexItem(flexItem))
    {
      // FIXME: If the min value is expected to be valid here, we need to come up with a non optional version
      // of computeMainAxisExtentForFlexItem and ensure it's valid through the virtual calls of
      // computeIntrinsicLogicalContentHeightUsing.
      LayoutUnit contentSize;
      auto &flexItemCrossSizeLength = PreferredCrossSizeLengthForFlexItem(flexItem);

      bool canComputeSizeThroughAspectRatio =
        flexItem.IsRenderReplaced() && FlexItemHasComputableAspectRatio(flexItem)
        && FlexItemCrossSizeIsDefinite(flexItem, flexItemCrossSizeLength);

      if (canComputeSizeThroughAspectRatio)
      {
        contentSize = ComputeMainSizeFromAspectRatioUsing(flexItem, flexItemCrossSizeLength);
      }
      else
      {
        contentSize =
          ComputeMainAxisExtentForFlexItem(flexItem, style::MinimumSize {css::keywords::MinContent {}})
            .value_or(0_lu);
      }

      if (FlexItemHasAspectRatio(flexItem))
      {
        contentSize = AdjustFlexItemSizeForAspectRatioCrossAxisMinAndMax(flexItem, contentSize);
      }
      krys_debug_assert(contentSize >= 0);
      contentSize = std::min(contentSize, maxExtent.value_or(contentSize));

      auto mainSize = PreferredMainSizeLengthForFlexItem(flexItem);
      if (FlexItemMainSizeIsDefinite(flexItem, mainSize))
      {
        LayoutUnit resolvedMainSize = ComputeMainAxisExtentForFlexItem(flexItem, mainSize).value_or(0_lu);
        krys_debug_assert(resolvedMainSize >= 0);
        LayoutUnit specifiedSize = std::min(resolvedMainSize, maxExtent.value_or(resolvedMainSize));
        return {std::min(specifiedSize, contentSize), maxExtent.value_or(LayoutUnit::Max())};
      }

      if (flexItem.IsRenderReplaced()
          && FlexItemHasComputableAspectRatioAndCrossSizeIsConsideredDefinite(flexItem))
      {
        LayoutUnit transferredSize = ComputeMainSizeFromAspectRatioUsing(flexItem, flexItemCrossSizeLength);
        transferredSize = AdjustFlexItemSizeForAspectRatioCrossAxisMinAndMax(flexItem, transferredSize);
        return {std::min(transferredSize, contentSize), maxExtent.value_or(LayoutUnit::Max())};
      }

      return {contentSize, maxExtent.value_or(LayoutUnit::Max())};
    }

    return {0_lu, maxExtent.value_or(LayoutUnit::Max())};
  }

  bool RenderFlexibleBox::CanUseFlexItemForPercentageResolution(const RenderBox &flexItem) noexcept
  {
    krys_debug_assert(flexItem.IsFlexItem());

    auto canUseByLayoutPhase = [&]
    {
      if (_inFlexItemIntrinsicWidthComputation)
      {
        return FlexItemCrossSizeShouldUseContainerCrossSize(flexItem) && !IsFlexItem();
      }

      if (_afterMainAxisItemSizing)
      {
        // Final sizes for flex items are available only along the main axis.
        // Percentages can be resolved only against those items when they are orthogonal to the flex container
        // (i.e., their logical height is computed and final)
        return !MainAxisIsFlexItemInlineAxis(flexItem);
      }

      if (_afterCrossAxisItemSizing)
      {
        // Final sizes for flex items are known in both the main and cross directions, so it's fine to resolve
        // percentage heights using those final values.
        return true;
      }

      if (_inPostFlexUpdateScrollbarLayout)
      {
        // We run layout on flex content _after_ performing flex layout (see
        // endAndCommitUpdateScrollInfoAfterLayoutTransaction/updateScrollInfoAfterLayout). Final sizes for
        // flex items are known in both the main and cross directions.
        return true;
      }

      if (_inSimplifiedLayout)
      {
        // While in simplified layout, we should only re-compute overflow and/or re-position out-of-flow
        // boxes, some renderers (e.g. RenderReplaced and subclasses) currently ignore this optimization and
        // run regular layout. Final sizes for flex items are known in both the main and cross directions,
        // computed during previous layout(s).
        return true;
      }

      if (&flexItem == View().FrameView().LayoutContext().SubtreeLayoutRoot())
      {
        return !MainAxisIsFlexItemInlineAxis(flexItem);
      }

      // Outside of layout (i.e. when using relative percentage positioning), base the decision on style.
      return !_inLayout;
    };
    if (!canUseByLayoutPhase())
    {
      return false;
    }

    auto canUseByStyle = [&]
    {
      if (MainAxisIsFlexItemInlineAxis(flexItem))
      {
        return AlignmentForFlexItem(flexItem) == ItemPosition::Stretch;
      }

      if (flexItem.Style().GetFlexGrow() == style::ComputedStyle::InitialFlexGrow()
          && flexItem.Style().GetFlexShrink().IsZero()
          && FlexItemMainSizeIsDefinite(flexItem, FlexBasisForFlexItem(flexItem)))
      {
        return true;
      }

      return CanComputePercentageFlexBasis(flexItem,
                                           style::PreferredSize {style::PreferredSize::Percentage {0}},
                                           UpdatePercentageHeightDescendants::Yes);
    };
    return canUseByStyle();
  }

  // This method is only called whenever a descendant of a flex item wants to resolve a percentage in its
  // block axis (logical height). The key here is that percentages should be generally resolved before the
  // flex item is flexed, meaning that they shouldn't be recomputed once the flex item has been flexed. There
  // are some exceptions though that are implemented here, like the case of fully inflexible items with
  // definite flex-basis, or whenever the flex container has a definite main size. See
  // https://drafts.csswg.org/css-flexbox/#definite-sizes for additional details.
  Maybe<LayoutUnit> RenderFlexibleBox::UsedFlexItemOverridingLogicalHeightForPercentageResolution(
    const RenderBox &flexItem) noexcept
  {
    return CanUseFlexItemForPercentageResolution(flexItem) ? flexItem.OverridingBorderBoxLogicalHeight()
                                                           : null;
  }

  LayoutUnit
    RenderFlexibleBox::AdjustFlexItemSizeForAspectRatioCrossAxisMinAndMax(const RenderBox &flexItem,
                                                                          LayoutUnit flexItemSize) noexcept
  {
    auto &crossMin = MinCrossSizeLengthForFlexItem(flexItem);
    auto &crossMax = MaxCrossSizeLengthForFlexItem(flexItem);

    if (FlexItemCrossSizeIsDefinite(flexItem, crossMax))
    {
      LayoutUnit maxValue = ComputeMainSizeFromAspectRatioUsing(flexItem, crossMax);
      flexItemSize = std::min(maxValue, flexItemSize);
    }

    if (FlexItemCrossSizeIsDefinite(flexItem, crossMin))
    {
      LayoutUnit minValue = ComputeMainSizeFromAspectRatioUsing(flexItem, crossMin);
      flexItemSize = std::max(minValue, flexItemSize);
    }

    return flexItemSize;
  }

  void RenderFlexibleBox::MaybeCacheFlexItemMainIntrinsicSize(RenderBox &flexItem,
                                                              RelayoutChildren relayoutChildren) noexcept
  {
    if (!FlexItemHasIntrinsicMainAxisSize(flexItem))
    {
      return;
    }

    // If this condition is true, then computeMainAxisExtentForFlexItem will call
    // flexItem.intrinsicContentLogicalHeight() and flexItem.scrollbarLogicalHeight(),
    // so if the child has intrinsic min/max/preferred size, run layout on it now to make sure
    // its logical height and scroll bars are up to date.
    UpdateBlockChildDirtyBitsBeforeLayout(relayoutChildren, flexItem);
    // Don't resolve percentages in children. This is especially important for the min-height calculation,
    // where we want percentages to be treated as auto. For flex-basis itself, this is not a problem because
    // by definition we have an indefinite flex basis here and thus percentages should not resolve.
    if (flexItem.NeedsLayout() || !_intrinsicSizeAlongMainAxis.contains(&flexItem))
    {
      auto percentResolveDisableScope =
        FlexPercentResolveDisabler {View().FrameView().LayoutContext(), flexItem};
      flexItem.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      flexItem.LayoutIfNeeded();
      CacheFlexItemMainSize(flexItem);
    }
  }

  RenderFlexibleBox::FlexLayoutItem
    RenderFlexibleBox::ConstructFlexLayoutItem(RenderBox &flexItem,
                                               RelayoutChildren relayoutChildren) noexcept
  {
    auto everHadLayout = flexItem.EverHadLayout();
    flexItem.ClearOverridingSize();
    if (auto *flexibleBox = DynamicDowncast<RenderFlexibleBox>(flexItem))
    {
      flexibleBox->ResetHasDefiniteHeight();
    }

    if (everHadLayout && flexItem.HasTrimmedMargin(Maybe<style::MarginTrimSide> {}))
    {
      flexItem.ClearTrimmedMarginsMarkings();
    }

    if (flexItem.ShouldInvalidatePreferredWidths())
    {
      flexItem.SetNeedsPreferredWidthsUpdate(RenderObjectMarkingBehavior::MarkOnlyThis);
    }

    LayoutUnit borderAndPadding = IsHorizontalFlow() ? flexItem.HorizontalBorderAndPaddingExtent()
                                                     : flexItem.VerticalBorderAndPaddingExtent();
    LayoutUnit innerFlexBaseSize =
      ComputeFlexBaseSizeForFlexItem(flexItem, borderAndPadding, relayoutChildren);
    LayoutUnit margin =
      IsHorizontalFlow() ? flexItem.HorizontalMarginExtent() : flexItem.VerticalMarginExtent();
    return FlexLayoutItem(flexItem, innerFlexBaseSize, borderAndPadding, margin,
                          ComputeFlexItemMinMaxSizes(flexItem), everHadLayout);
  }

  void RenderFlexibleBox::FreezeViolations(List<FlexLayoutItem *> &violations, LayoutUnit &availableFreeSpace,
                                           double &totalFlexGrow, double &totalFlexShrink,
                                           double &totalWeightedFlexShrink) noexcept
  {
    for (auto *violation : violations)
    {
      krys_debug_assert(!violation->Frozen);
      auto &flexItemStyle = violation->Style();
      LayoutUnit flexItemSize = violation->FlexedContentSize;
      availableFreeSpace -= flexItemSize - violation->FlexBaseContentSize;
      totalFlexGrow -= flexItemStyle.GetFlexGrow().Value;
      totalFlexShrink -= flexItemStyle.GetFlexShrink().Value;
      totalWeightedFlexShrink -=
        flexItemStyle.GetFlexShrink().Value * violation->FlexBaseContentSize.ToDouble();
      // totalWeightedFlexShrink can be negative when we exceed the precision of
      // a double when we initially calcuate totalWeightedFlexShrink. We then
      // subtract each child's weighted flex shrink with full precision, now
      // leading to a negative result. See
      // css3/flexbox/large-flex-shrink-assert.html
      totalWeightedFlexShrink = std::max(totalWeightedFlexShrink, 0.0);
      violation->Frozen = true;
    }
  }

  void RenderFlexibleBox::FreezeInflexibleItems(FlexSign flexSign, FlexLayoutItems &flexLayoutItems,
                                                LayoutUnit &remainingFreeSpace, double &totalFlexGrow,
                                                double &totalFlexShrink,
                                                double &totalWeightedFlexShrink) noexcept
  {
    // Per https://drafts.csswg.org/css-flexbox/#resolve-flexible-lengths step 2,
    // we freeze all items with a flex factor of 0 as well as those with a min/max
    // size violation.
    List<FlexLayoutItem *> newInflexibleItems;
    for (auto &flexLayoutItem : flexLayoutItems)
    {
      krys_debug_assert(!flexLayoutItem.Renderer->IsOutOfFlowPositioned());
      krys_debug_assert(!flexLayoutItem.Frozen);
      float flexFactor = (flexSign == FlexSign::PositiveFlexibility)
                           ? flexLayoutItem.Style().GetFlexGrow().Value
                           : flexLayoutItem.Style().GetFlexShrink().Value;
      if (!flexFactor
          || (flexSign == FlexSign::PositiveFlexibility
              && flexLayoutItem.FlexBaseContentSize > flexLayoutItem.HypotheticalMainContentSize)
          || (flexSign == FlexSign::NegativeFlexibility
              && flexLayoutItem.FlexBaseContentSize < flexLayoutItem.HypotheticalMainContentSize))
      {
        flexLayoutItem.FlexedContentSize = flexLayoutItem.HypotheticalMainContentSize;
        newInflexibleItems.push_back(&flexLayoutItem);
      }
    }
    FreezeViolations(newInflexibleItems, remainingFreeSpace, totalFlexGrow, totalFlexShrink,
                     totalWeightedFlexShrink);
  }

  // Returns true if we successfully ran the algorithm and sized the flex items.
  bool RenderFlexibleBox::ResolveFlexibleLengths(FlexSign flexSign, FlexLayoutItems &flexLayoutItems,
                                                 LayoutUnit initialFreeSpace, LayoutUnit &remainingFreeSpace,
                                                 double &totalFlexGrow, double &totalFlexShrink,
                                                 double &totalWeightedFlexShrink) noexcept
  {
    LayoutUnit totalViolation;
    LayoutUnit usedFreeSpace;
    List<FlexLayoutItem *> minViolations;
    List<FlexLayoutItem *> maxViolations;

    double sumFlexFactors = (flexSign == FlexSign::PositiveFlexibility) ? totalFlexGrow : totalFlexShrink;
    if (sumFlexFactors > 0 && sumFlexFactors < 1)
    {
      LayoutUnit fractional(initialFreeSpace.ToDouble() * sumFlexFactors);
      if (fractional.Abs() < remainingFreeSpace.Abs())
      {
        remainingFreeSpace = fractional;
      }
    }

    for (auto &flexLayoutItem : flexLayoutItems)
    {
      // This check also covers out-of-flow children.
      if (flexLayoutItem.Frozen)
      {
        continue;
      }

      auto &flexItemStyle = flexLayoutItem.Style();
      LayoutUnit flexItemSize = flexLayoutItem.FlexBaseContentSize;
      double extraSpace = 0;
      if (remainingFreeSpace > 0 && totalFlexGrow > 0 && flexSign == FlexSign::PositiveFlexibility
          && std::isfinite(totalFlexGrow))
      {
        extraSpace = remainingFreeSpace.ToDouble() * flexItemStyle.GetFlexGrow().Value / totalFlexGrow;
      }
      else if (remainingFreeSpace < 0 && totalWeightedFlexShrink > 0
               && flexSign == FlexSign::NegativeFlexibility && std::isfinite(totalWeightedFlexShrink)
               && !flexItemStyle.GetFlexShrink().IsZero())
      {
        extraSpace = remainingFreeSpace.ToDouble() * flexItemStyle.GetFlexShrink().Value
                     * flexLayoutItem.FlexBaseContentSize.ToDouble() / totalWeightedFlexShrink;
      }
      if (std::isfinite(extraSpace))
      {
        flexItemSize += LayoutUnit::FromFloatRound(static_cast<float>(extraSpace));
      }

      LayoutUnit adjustedFlexItemSize = flexLayoutItem.ConstrainSizeByMinMax(flexItemSize);
      krys_debug_assert(adjustedFlexItemSize >= 0);
      flexLayoutItem.FlexedContentSize = adjustedFlexItemSize;
      usedFreeSpace += adjustedFlexItemSize - flexLayoutItem.FlexBaseContentSize;

      LayoutUnit violation = adjustedFlexItemSize - flexItemSize;
      if (violation > 0)
      {
        minViolations.push_back(&flexLayoutItem);
      }
      else if (violation < 0)
      {
        maxViolations.push_back(&flexLayoutItem);
      }
      totalViolation += violation;
    }

    if (totalViolation)
    {
      FreezeViolations(totalViolation < 0 ? maxViolations : minViolations, remainingFreeSpace, totalFlexGrow,
                       totalFlexShrink, totalWeightedFlexShrink);
    }
    else
    {
      remainingFreeSpace -= usedFreeSpace;
    }

    return !totalViolation;
  }

  void RenderFlexibleBox::SetOverridingMainSizeForFlexItem(RenderBox &flexItem,
                                                           LayoutUnit preferredSize) noexcept
  {
    if (MainAxisIsFlexItemInlineAxis(flexItem))
    {
      flexItem.SetOverridingBorderBoxLogicalWidth(preferredSize + flexItem.BorderAndPaddingLogicalWidth());
    }
    else
    {
      flexItem.SetOverridingBorderBoxLogicalHeight(preferredSize + flexItem.BorderAndPaddingLogicalHeight());
    }
  }

  LayoutUnit
    RenderFlexibleBox::StaticMainAxisPositionForPositionedFlexItem(const RenderBox &flexItem) noexcept
  {
    auto flexItemMainExtent = MainAxisMarginExtentForFlexItem(flexItem) + MainAxisExtentForFlexItem(flexItem);
    auto availableSpace = MainAxisContentExtent(ContentBoxLogicalHeight()) - flexItemMainExtent;
    auto isReverse = IsColumnOrRowReverse();
    LayoutUnit offset = InitialJustifyContentOffset(Style(), availableSpace, {}, isReverse);
    if (isReverse)
    {
      offset = availableSpace - offset;
    }
    return offset;
  }

  LayoutUnit
    RenderFlexibleBox::StaticCrossAxisPositionForPositionedFlexItem(const RenderBox &flexItem) noexcept
  {
    auto availableSpace = AvailableAlignmentSpaceForFlexItem(CrossAxisContentExtent(), flexItem);
    auto safety = OverflowAlignmentForFlexItem(flexItem);
    auto align = AlignmentForFlexItem(flexItem);
    if (availableSpace < 0 && safety == OverflowAlignment::Safe)
    {
      align = ItemPosition::FlexStart;
    }
    return AlignmentOffset(availableSpace, align, {}, {}, IsWrapReverse());
  }

  LayoutUnit RenderFlexibleBox::StaticInlinePositionForPositionedFlexItem(const RenderBox &flexItem) noexcept
  {
    return StartOffsetForContent()
           + (IsColumnFlow() ? StaticCrossAxisPositionForPositionedFlexItem(flexItem)
                             : StaticMainAxisPositionForPositionedFlexItem(flexItem));
  }

  LayoutUnit RenderFlexibleBox::StaticBlockPositionForPositionedFlexItem(const RenderBox &flexItem) noexcept
  {
    return BorderAndPaddingBefore()
           + (IsColumnFlow() ? StaticMainAxisPositionForPositionedFlexItem(flexItem)
                             : StaticCrossAxisPositionForPositionedFlexItem(flexItem));
  }

  bool RenderFlexibleBox::SetStaticPositionForPositionedLayout(const RenderBox &flexItem) noexcept
  {
    bool positionChanged = false;
    auto *layer = flexItem.Layer();
    if (flexItem.Style().HasStaticInlinePosition(GetWritingMode().IsHorizontal()))
    {
      LayoutUnit inlinePosition = StaticInlinePositionForPositionedFlexItem(flexItem);
      if (layer->StaticInlinePosition() != inlinePosition)
      {
        layer->SetStaticInlinePosition(inlinePosition);
        positionChanged = true;
      }
    }
    if (flexItem.Style().HasStaticBlockPosition(GetWritingMode().IsHorizontal()))
    {
      LayoutUnit blockPosition = StaticBlockPositionForPositionedFlexItem(flexItem);
      if (layer->StaticBlockPosition() != blockPosition)
      {
        layer->SetStaticBlockPosition(blockPosition);
        positionChanged = true;
      }
    }
    return positionChanged;
  }

  // This refers to https://drafts.csswg.org/css-flexbox-1/#definite-sizes, section 1).
  LayoutUnit RenderFlexibleBox::ComputeCrossSizeForFlexItemUsingContainerCrossSize(
    const RenderBox &flexItem) const noexcept
  {
    if (IsColumnFlow())
    {
      return ContentBoxLogicalWidth();
    }

    // Keep this sync'ed with flexItemCrossSizeShouldUseContainerCrossSize().
    auto definiteSizeValue = [&]
    {
      // Let's compute the definite size value for the flex item (value that we can resolve without running
      // layout).
      auto isHorizontal = IsHorizontalFlow();
      auto &size = isHorizontal ? Style().Height() : Style().Width();
      krys_debug_assert(size.IsFixed()
                        || (size.IsPercent() && AvailableLogicalHeightForPercentageComputation()));
      LayoutUnit definiteValue;
      if (auto fixedSize = size.TryFixed())
      {
        definiteValue = LayoutUnit {fixedSize->ResolveZoom(Style().UsedZoomForLength())};
      }
      else if (size.IsPercent())
      {
        definiteValue = AvailableLogicalHeightForPercentageComputation().value_or(0_lu);
      }

      auto &maximumSize = isHorizontal ? Style().MaxHeight() : Style().MaxWidth();
      if (auto fixedMaximumSize = maximumSize.TryFixed())
      {
        definiteValue =
          std::min(definiteValue, LayoutUnit {fixedMaximumSize->ResolveZoom(Style().UsedZoomForLength())});
      }

      auto &minimumSize = isHorizontal ? Style().MinHeight() : Style().MinWidth();
      if (auto fixedMinimumSize = minimumSize.TryFixed())
      {
        definiteValue =
          std::max(definiteValue, LayoutUnit {fixedMinimumSize->ResolveZoom(Style().UsedZoomForLength())});
      }

      return definiteValue;
    };
    return std::max(0_lu, definiteSizeValue() - CrossAxisMarginExtentForFlexItem(flexItem));
  }

  void RenderFlexibleBox::PrepareFlexItemForPositionedLayout(RenderBox &flexItem) noexcept
  {
    krys_debug_assert(flexItem.IsOutOfFlowPositioned());
    flexItem.ContainingBlock()->AddOutOfFlowBox(flexItem);
    auto *layer = flexItem.Layer();
    LayoutUnit staticInlinePosition = FlowAwareBorderStart() + FlowAwarePaddingStart();
    if (layer->StaticInlinePosition() != staticInlinePosition)
    {
      layer->SetStaticInlinePosition(staticInlinePosition);
      if (flexItem.Style().HasStaticInlinePosition(GetWritingMode().IsHorizontal()))
      {
        flexItem.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }
    }

    LayoutUnit staticBlockPosition = FlowAwareBorderBefore() + FlowAwarePaddingBefore();
    if (layer->StaticBlockPosition() != staticBlockPosition)
    {
      layer->SetStaticBlockPosition(staticBlockPosition);
      if (flexItem.Style().HasStaticBlockPosition(GetWritingMode().IsHorizontal()))
      {
        flexItem.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }
    }
  }

  OverflowAlignment RenderFlexibleBox::OverflowAlignmentForFlexItem(const RenderBox &flexItem) const noexcept
  {
    auto normalBehavior = SelfAlignmentNormalBehavior();
    if (!flexItem.Style().GetAlignSelf().IsAuto())
    {
      return flexItem.Style().GetAlignSelf().Resolve(normalBehavior).Overflow();
    }
    return Style().GetAlignItems().Resolve(normalBehavior).Overflow();
  }

  ItemPosition RenderFlexibleBox::AlignmentForFlexItem(const RenderBox &flexItem) const noexcept
  {
    auto align = [&]
    {
      auto normalBehavior = SelfAlignmentNormalBehavior();
      if (!flexItem.Style().GetAlignSelf().IsAuto())
      {
        return flexItem.Style().GetAlignSelf().Resolve(normalBehavior).Position();
      }
      return Style().GetAlignItems().Resolve(normalBehavior).Position();
    }();

    krys_debug_assert(align != ItemPosition::Auto && align != ItemPosition::Normal);
    // Left and Right are only for justify-*.
    krys_debug_assert(align != ItemPosition::Left && align != ItemPosition::Right);

    // We can safely return here because start/end are not affected by a reversed flex-wrap because the
    // alignment container is the flex line, and in a wrap reversed flex container the start and end within
    // a flex line are still the same. Contrary to this flex-start/flex-end depend on the flex container
    // start/end edges which are flipped in the case of wrap-reverse.
    if (align == ItemPosition::Start)
    {
      return ItemPosition::FlexStart;
    }
    if (align == ItemPosition::End)
    {
      return ItemPosition::FlexEnd;
    }

    if (align == ItemPosition::SelfStart || align == ItemPosition::SelfEnd)
    {
      bool hasSameDirection =
        IsHorizontalFlow()
          ? GetWritingMode().IsAnyTopToBottom() == flexItem.GetWritingMode().IsAnyTopToBottom()
          : GetWritingMode().IsAnyLeftToRight() == flexItem.GetWritingMode().IsAnyLeftToRight();
      return hasSameDirection == (align == ItemPosition::SelfStart) ? ItemPosition::FlexStart
                                                                    : ItemPosition::FlexEnd;
    }

    if (IsWrapReverse())
    {
      if (align == ItemPosition::FlexStart)
      {
        align = ItemPosition::FlexEnd;
      }
      else if (align == ItemPosition::FlexEnd)
      {
        align = ItemPosition::FlexStart;
      }
    }

    return align;
  }

  void RenderFlexibleBox::ResetAutoMarginsAndLogicalTopInCrossAxis(RenderBox &flexItem) noexcept
  {
    if (HasAutoMarginsInCrossAxis(flexItem))
    {
      flexItem.UpdateLogicalHeight();
      if (IsHorizontalFlow())
      {
        if (flexItem.Style().MarginTop().IsAuto())
        {
          flexItem.SetMarginTop(0_lu);
        }
        if (flexItem.Style().MarginBottom().IsAuto())
        {
          flexItem.SetMarginBottom(0_lu);
        }
      }
      else
      {
        if (flexItem.Style().MarginLeft().IsAuto())
        {
          flexItem.SetMarginLeft(0_lu);
        }
        if (flexItem.Style().MarginRight().IsAuto())
        {
          flexItem.SetMarginRight(0_lu);
        }
      }
    }
  }

  bool RenderFlexibleBox::NeedToStretchFlexItemLogicalHeight(const RenderBox &flexItem) const noexcept
  {
    // This function is a little bit magical. It relies on the fact that blocks
    // intrinsically "stretch" themselves in their inline axis, i.e. a <div> has
    // an implicit width: 100%. So the child will automatically stretch if our
    // cross axis is the child's inline axis. That's the case if:
    // - We are horizontal and the child is in vertical writing mode
    // - We are vertical and the child is in horizontal writing mode
    // Otherwise, we need to stretch if the cross axis size is auto.
    if (AlignmentForFlexItem(flexItem) != ItemPosition::Stretch)
    {
      return false;
    }

    if (IsHorizontalFlow() != flexItem.IsHorizontalWritingMode())
    {
      return false;
    }

    // Aspect ratio is properly handled by RenderReplaced during layout.
    if (flexItem.IsRenderReplaced() && FlexItemHasAspectRatio(flexItem))
    {
      return false;
    }

    return flexItem.Style().LogicalHeight().IsAuto();
  }

  bool RenderFlexibleBox::FlexItemHasIntrinsicMainAxisSize(const RenderBox &flexItem) noexcept
  {
    if (MainAxisIsFlexItemInlineAxis(flexItem))
    {
      return false;
    }

    auto flexBasis = FlexBasisForFlexItem(flexItem);
    auto &minSize = MinMainSizeLengthForFlexItem(flexItem);
    auto &maxSize = MaxMainSizeLengthForFlexItem(flexItem);
    // FIXME: we must run flexItemMainSizeIsDefinite() because it might end up calling
    // computePercentageLogicalHeight() which has some side effects like calling addPercentHeightDescendant()
    // for example so it is not possible to skip the call for example by moving it to the end of the
    // conditional expression. This is error-prone and we should refactor computePercentageLogicalHeight() at
    // some point so that it only computes stuff without those side effects.
    if (!FlexItemMainSizeIsDefinite(flexItem, flexBasis) || minSize.IsIntrinsic() || maxSize.IsIntrinsic())
    {
      return true;
    }

    if (ShouldApplyMinSizeAutoForFlexItem(flexItem))
    {
      return true;
    }

    return false;
  }

  Overflow RenderFlexibleBox::MainAxisOverflowForFlexItem(const RenderBox &flexItem) const noexcept
  {
    if (IsHorizontalFlow())
    {
      return flexItem.Style().OverflowX();
    }
    return flexItem.Style().OverflowY();
  }

  Overflow RenderFlexibleBox::CrossAxisOverflowForFlexItem(const RenderBox &flexItem) const noexcept
  {
    if (IsHorizontalFlow())
    {
      return flexItem.Style().OverflowY();
    }
    return flexItem.Style().OverflowX();
  }

  bool RenderFlexibleBox::FlexItemHasPercentHeightDescendants(const RenderBox &renderer) const noexcept
  {
    // FIXME: This function can be removed soon after webkit.org/b/204318 is fixed. Evaluate whether the
    // skipContainingBlockForPercentHeightCalculation() check below should be moved to the caller in that
    // case.
    auto *renderBlock = DynamicDowncast<RenderBlock>(renderer);
    if (!renderBlock)
    {
      return false;
    }

    // FlexibleBoxImpl's like RenderButton might wrap their children in anonymous blocks. Those anonymous
    // blocks are skipped for percentage height calculations in RenderBox::computePercentageLogicalHeight()
    // and thus addPercentHeightDescendant() is never called for them. This means that this method would
    // always wrongly return false for a child of a <button> with a percentage height.
    if (HasPercentHeightDescendants()
        && SkipContainingBlockForPercentHeightCalculation(renderer, IsHorizontalWritingMode()
                                                                      != renderer.IsHorizontalWritingMode()))
    {
      for (auto *descendant : *PercentHeightDescendants())
      {
        if (renderBlock->IsContainingBlockAncestorFor(*descendant))
        {
          return true;
        }
      }
    }

    if (!renderBlock->HasPercentHeightDescendants())
    {
      return false;
    }

    auto *percentHeightDescendants = renderBlock->PercentHeightDescendants();
    if (!percentHeightDescendants)
    {
      return false;
    }

    for (auto *descendant : *percentHeightDescendants)
    {
      bool hasOutOfFlowAncestor = false;
      for (auto *ancestor = descendant->ContainingBlock(); ancestor && ancestor != renderBlock;
           ancestor = ancestor->ContainingBlock())
      {
        if (ancestor->IsOutOfFlowPositioned())
        {
          hasOutOfFlowAncestor = true;
          break;
        }
      }
      if (!hasOutOfFlowAncestor)
      {
        return true;
      }
    }
    return false;
  }

  void RenderFlexibleBox::LayoutAndPlaceFlexItems(
    LayoutUnit &crossAxisOffset, FlexLayoutItems &flexLayoutItems, LayoutUnit availableFreeSpace,
    RelayoutChildren relayoutChildren, FlexLineStates &lineStates, LayoutUnit gapBetweenItems) noexcept
  {
    LayoutUnit autoMarginOffset = AutoMarginOffsetInMainAxis(flexLayoutItems, availableFreeSpace);
    LayoutUnit mainAxisOffset = FlowAwareBorderStart() + FlowAwarePaddingStart();
    mainAxisOffset += InitialJustifyContentOffset(
      Style(), availableFreeSpace, static_cast<uint32>(flexLayoutItems.size()), IsColumnOrRowReverse());
    if (Style().GetFlexDirection() == FlexDirection::RowReverse)
    {
      mainAxisOffset +=
        LayoutUnit(IsHorizontalFlow() ? VerticalScrollbarWidth() : HorizontalScrollbarHeight());
    }

    if (availableFreeSpace < 0)
    {
      auto resolvedJustifyContent = Style().GetJustifyContent().Resolve(ContentAlignmentNormalBehavior());
      auto distribution = resolvedJustifyContent.Distribution();
      auto safety = resolvedJustifyContent.Overflow();
      auto position = ResolveLeftRightAlignment(resolvedJustifyContent.Position(), resolvedJustifyContent,
                                                Style(), IsColumnOrRowReverse());
      LayoutUnit overflow = ContentAlignmentStartOverflow(availableFreeSpace, position, distribution, safety,
                                                          IsColumnOrRowReverse());
      _justifyContentStartOverflow = std::max(_justifyContentStartOverflow, overflow);
    }

    LayoutUnit totalMainExtent = MainAxisExtent();
    LayoutUnit maxFlexItemCrossAxisExtent;

    LayoutUnit maxAscent;
    LayoutUnit maxDescent = LayoutUnit::Min();
    LayoutUnit lastBaselineMaxAscent;
    Maybe<BaselineAlignmentState> baselineAlignmentState;

    auto resolvedJustifyContent = Style().GetJustifyContent().Resolve(ContentAlignmentNormalBehavior());
    auto distribution = resolvedJustifyContent.Distribution();
    bool shouldFlipMainAxis = !IsColumnFlow() && !IsLeftToRightFlow();
    for (size_t i = 0; i < flexLayoutItems.size(); ++i)
    {
      auto &flexLayoutItem = flexLayoutItems[i];
      auto &flexItem = *flexLayoutItem.Renderer;

      krys_debug_assert(!flexItem.IsOutOfFlowPositioned());

      SetOverridingMainSizeForFlexItem(flexItem, flexLayoutItem.FlexedContentSize);
      // The flexed content size and the override size include the scrollbar
      // width, so we need to compare to the size including the scrollbar.
      // FIXME: Should it include the scrollbar?
      if (flexLayoutItem.FlexedContentSize != MainAxisContentExtentForFlexItemIncludingScrollbar(flexItem))
      {
        flexItem.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }
      else
      {
        // To avoid double applying margin changes in
        // updateAutoMarginsInCrossAxis, we reset the margins here.
        ResetAutoMarginsAndLogicalTopInCrossAxis(flexItem);
      }
      // We may have already forced relayout for orthogonal flowing children in
      // computeInnerFlexBaseSizeForFlexItem.
      bool forceFlexItemRelayout =
        relayoutChildren == RelayoutChildren::Yes && !_relaidOutFlexItems.contains(&flexItem);
      if (!forceFlexItemRelayout && FlexItemHasPercentHeightDescendants(flexItem))
      {
        // Have to force another relayout even though the child is sized
        // correctly, because its descendants are not sized correctly yet. Our
        // previous layout of the child was done without an override height set.
        // So, redo it here.
        forceFlexItemRelayout = true;
      }
      UpdateFlexItemDirtyBitsBeforeLayout(forceFlexItemRelayout, flexItem);
      // FIXME: Mark the item for pagination relayout (markForPaginationRelayoutIfNeeded), once there's
      // pagination.
      if (flexItem.NeedsLayout())
      {
        _relaidOutFlexItems.insert(&flexItem);
      }
      {
        SetForScope flexLayoutScope(_afterMainAxisItemSizing, true);
        flexItem.LayoutIfNeeded();
      }
      // FIXME: Repaint items that had never been laid out (checkForRepaintDuringLayout), once there's
      // painting.

      UpdateAutoMarginsInMainAxis(flexItem, autoMarginOffset);

      LayoutUnit flexItemCrossAxisMarginBoxExtent;

      auto alignment = AlignmentForFlexItem(flexItem);
      if ((alignment == ItemPosition::Baseline || alignment == ItemPosition::LastBaseline)
          && !HasAutoMarginsInCrossAxis(flexItem))
      {
        LayoutUnit ascent = MarginBoxAscentForFlexItem(flexItem);
        LayoutUnit descent =
          (CrossAxisMarginExtentForFlexItem(flexItem) + CrossAxisExtentForFlexItem(flexItem)) - ascent;
        maxDescent = std::max(maxDescent, descent);

        if (!baselineAlignmentState)
        {
          auto alignmentContextAxis =
            Style().IsRowFlexDirection() ? LogicalBoxAxis::Inline : LogicalBoxAxis::Block;
          baselineAlignmentState.emplace(flexItem, alignment, ascent, alignmentContextAxis,
                                         Style().GetWritingMode());
        }
        else
        {
          baselineAlignmentState->UpdateSharedGroup(flexItem, alignment, ascent);
        }

        if (alignment == ItemPosition::Baseline)
        {
          maxAscent = std::max(maxAscent, ascent);
          flexItemCrossAxisMarginBoxExtent = maxAscent + maxDescent;
        }
        else
        {
          lastBaselineMaxAscent = std::max(lastBaselineMaxAscent, ascent);
          flexItemCrossAxisMarginBoxExtent = lastBaselineMaxAscent + maxDescent;
        }
      }
      else
      {
        flexItemCrossAxisMarginBoxExtent =
          CrossAxisIntrinsicExtentForFlexItem(flexItem) + CrossAxisMarginExtentForFlexItem(flexItem);
      }

      if (!IsColumnFlow())
      {
        SetLogicalHeight(
          std::max(LogicalHeight(), crossAxisOffset + FlowAwareBorderAfter() + FlowAwarePaddingAfter()
                                      + flexItemCrossAxisMarginBoxExtent + CrossAxisScrollbarExtent()));
      }
      maxFlexItemCrossAxisExtent = std::max(maxFlexItemCrossAxisExtent, flexItemCrossAxisMarginBoxExtent);

      mainAxisOffset += FlowAwareMarginStartForFlexItem(flexItem);

      LayoutUnit flexItemMainExtent = MainAxisExtentForFlexItem(flexItem);
      // In an RTL column situation, this will apply the margin-right/margin-end
      // on the left. This will be fixed later in flipForRightToLeftColumn.
      auto leadingScrollbarSize = GetWritingMode().IsInlineFlipped() && GetWritingMode().IsVertical()
                                    ? MainAxisScrollbarExtent()
                                    : LayoutUnit();
      LayoutPoint location(shouldFlipMainAxis
                             ? totalMainExtent - mainAxisOffset - flexItemMainExtent - leadingScrollbarSize
                             : mainAxisOffset,
                           crossAxisOffset + FlowAwareMarginBeforeForFlexItem(flexItem));
      SetFlowAwareLocationForFlexItem(flexItem, location);
      mainAxisOffset += flexItemMainExtent + FlowAwareMarginEndForFlexItem(flexItem);

      if (i != flexLayoutItems.size() - 1)
      {
        // The last item does not get extra space added.
        mainAxisOffset += JustifyContentSpaceBetweenFlexItems(availableFreeSpace, distribution,
                                                              static_cast<uint32>(flexLayoutItems.size()))
                          + gapBetweenItems;
      }

      // FIXME: Deal with pagination.
    }

    if (IsColumnFlow())
    {
      SetLogicalHeight(std::max(LogicalHeight(), mainAxisOffset + FlowAwareBorderEnd() + FlowAwarePaddingEnd()
                                                   + LayoutUnit(ScrollbarLogicalHeight())));
    }

    if (Style().GetFlexDirection() == FlexDirection::ColumnReverse)
    {
      // We have to do an extra pass for column-reverse to reposition the flex
      // items since the start depends on the height of the flexbox, which we
      // only know after we've positioned all the flex items.
      UpdateLogicalHeight();
      LayoutColumnReverse(flexLayoutItems, crossAxisOffset, availableFreeSpace, gapBetweenItems);
    }

    lineStates.emplace_back(crossAxisOffset, maxFlexItemCrossAxisExtent, krys::move(baselineAlignmentState),
                            krys::move(flexLayoutItems));
    crossAxisOffset += maxFlexItemCrossAxisExtent;
  }

  void RenderFlexibleBox::LayoutColumnReverse(const FlexLayoutItems &flexLayoutItems,
                                              LayoutUnit crossAxisOffset, LayoutUnit availableFreeSpace,
                                              LayoutUnit gapBetweenItems) noexcept
  {
    // This is similar to the logic in layoutAndPlaceFlexItems, except we place
    // the children starting from the end of the flexbox. We also don't need to
    // layout anything since we're just moving the children to a new position.
    LayoutUnit mainAxisOffset = LogicalHeight() - FlowAwareBorderEnd() - FlowAwarePaddingEnd();
    mainAxisOffset -= InitialJustifyContentOffset(
      Style(), availableFreeSpace, static_cast<uint32>(flexLayoutItems.size()), IsColumnOrRowReverse());
    mainAxisOffset -= LayoutUnit(IsHorizontalFlow() ? VerticalScrollbarWidth() : HorizontalScrollbarHeight());

    auto distribution = Style().GetJustifyContent().Resolve(ContentAlignmentNormalBehavior()).Distribution();

    for (size_t i = 0; i < flexLayoutItems.size(); ++i)
    {
      auto &flexItem = *flexLayoutItems[i].Renderer;
      krys_debug_assert(!flexItem.IsOutOfFlowPositioned());
      mainAxisOffset -= MainAxisExtentForFlexItem(flexItem) + FlowAwareMarginEndForFlexItem(flexItem);
      SetFlowAwareLocationForFlexItem(
        flexItem, LayoutPoint(mainAxisOffset, crossAxisOffset + FlowAwareMarginBeforeForFlexItem(flexItem)));
      mainAxisOffset -= FlowAwareMarginStartForFlexItem(flexItem);

      if (i != flexLayoutItems.size() - 1)
      {
        // The last item does not get extra space added.
        mainAxisOffset -= JustifyContentSpaceBetweenFlexItems(availableFreeSpace, distribution,
                                                              static_cast<uint32>(flexLayoutItems.size()))
                          + gapBetweenItems;
      }
    }
  }

  void RenderFlexibleBox::AlignFlexLines(FlexLineStates &lineStates, LayoutUnit gapBetweenLines) noexcept
  {
    if (lineStates.empty() || !IsMultiline())
    {
      return;
    }

    auto alignedContent = Style().GetAlignContent().Resolve(ContentAlignmentNormalBehavior());
    auto position = alignedContent.Position();
    auto distribution = alignedContent.Distribution();
    auto safety = alignedContent.Overflow();

    bool isWrapReverse = IsWrapReverse();

    if (position == ContentPosition::FlexStart && !gapBetweenLines && safety != OverflowAlignment::Safe
        && !isWrapReverse)
    {
      return;
    }

    size_t numLines = lineStates.size();
    LayoutUnit availableCrossAxisSpace =
      CrossAxisContentExtent() - static_cast<int32>(numLines - 1) * gapBetweenLines;
    for (auto &lineState : lineStates)
    {
      availableCrossAxisSpace -= lineState.CrossAxisExtent;
    }

    _alignContentStartOverflow =
      ContentAlignmentStartOverflow(availableCrossAxisSpace, position, distribution, safety, isWrapReverse);
    LayoutUnit lineOffset = InitialAlignContentOffset(availableCrossAxisSpace, position, distribution, safety,
                                                      static_cast<uint32>(numLines), isWrapReverse);
    for (auto &lineState : lineStates)
    {
      lineState.CrossAxisOffset += lineOffset;
      for (auto &flexLayoutItem : lineState.FlexLayoutItems)
      {
        AdjustAlignmentForFlexItem(*flexLayoutItem.Renderer, lineOffset);
      }

      if (distribution == ContentDistribution::Stretch && availableCrossAxisSpace > 0)
      {
        lineState.CrossAxisExtent += availableCrossAxisSpace / static_cast<uint32>(numLines);
      }

      lineOffset += AlignContentSpaceBetweenFlexItems(availableCrossAxisSpace, distribution,
                                                      static_cast<uint32>(numLines))
                    + gapBetweenLines;
    }
  }

  void RenderFlexibleBox::AdjustAlignmentForFlexItem(RenderBox &flexItem, LayoutUnit delta) noexcept
  {
    krys_debug_assert(!flexItem.IsOutOfFlowPositioned());
    SetFlowAwareLocationForFlexItem(flexItem,
                                    FlowAwareLocationForFlexItem(flexItem) + LayoutSize(0_lu, delta));
  }

  void RenderFlexibleBox::AlignFlexItems(FlexLineStates &lineStates) noexcept
  {
    for (auto &lineState : lineStates)
    {
      LayoutUnit lineCrossAxisExtent = lineState.CrossAxisExtent;

      if (lineState.BaselineAlignment)
      {
        PerformBaselineAlignment(lineState);
      }

      for (auto &flexLayoutItem : lineState.FlexLayoutItems)
      {
        auto &flexItem = *flexLayoutItem.Renderer;
        krys_debug_assert(!flexItem.IsOutOfFlowPositioned());

        auto safety = OverflowAlignmentForFlexItem(flexItem);
        auto position = AlignmentForFlexItem(flexItem);
        if (UpdateAutoMarginsInCrossAxis(
              flexItem, std::max(0_lu, AvailableAlignmentSpaceForFlexItem(lineCrossAxisExtent, flexItem)))
            || position == ItemPosition::Baseline || position == ItemPosition::LastBaseline)
        {
          continue;
        }

        if (position == ItemPosition::Stretch)
        {
          ApplyStretchAlignmentToFlexItem(flexItem, lineCrossAxisExtent);
        }
        LayoutUnit availableSpace = AvailableAlignmentSpaceForFlexItem(lineCrossAxisExtent, flexItem);
        if (availableSpace < 0 && safety == OverflowAlignment::Safe)
        {
          position = ItemPosition::FlexStart; // See Start == FlexStart assumption in alignmentForFlexItem().
        }
        LayoutUnit offset = AlignmentOffset(availableSpace, position, {}, {}, IsWrapReverse());
        AdjustAlignmentForFlexItem(flexItem, offset);
      }
    }
  }

  void RenderFlexibleBox::PerformBaselineAlignment(LineState &lineState) noexcept
  {
    krys_debug_assert(lineState.BaselineAlignment.has_value());

    auto lineCrossAxisExtent = lineState.CrossAxisExtent;
    bool containerHasWrapReverse = IsWrapReverse();

    auto flexItemWritingModeForBaselineAlignment = [&](const RenderBox &flexItem)
    {
      if (MainAxisIsFlexItemInlineAxis(flexItem))
      {
        return flexItem.Style().GetWritingMode();
      }

      auto alignmentContextAxis =
        Style().IsRowFlexDirection() ? LogicalBoxAxis::Inline : LogicalBoxAxis::Block;
      return BaselineAlignmentState::UsedWritingModeForBaselineAlignment(
        alignmentContextAxis, GetWritingMode(), flexItem.GetWritingMode());
    };

    auto shouldAdjustItemTowardsCrossAxisEnd =
      [&](FlowDirection flexItemBlockFlowDirection, ItemPosition alignment)
    {
      krys_debug_assert(alignment == ItemPosition::Baseline || alignment == ItemPosition::LastBaseline);

      // For first baseline aligned items, if its block direction is the opposite of
      // the cross axis direction, then that means its fallback alignment (safe self-start)
      // is in the direction of the end of the cross axis
      //
      // For last baseline aligned items, if its block direction is in the same direction as
      // the cross axis direction, then that means its fallback alignment (safe self-end) is
      // in the direction of the end of the cross axis
      if (alignment == ItemPosition::Baseline)
      {
        return CrossAxisDirection() != flexItemBlockFlowDirection;
      }
      return CrossAxisDirection() == flexItemBlockFlowDirection;
    };

    for (auto &baselineSharingGroup : lineState.BaselineAlignment->SharedGroups())
    {
      LayoutUnit minMarginAfterBaseline = LayoutUnit::Max();
      // The group only hands out const pointers; the items are this container's children.
      for (const auto *constFlexItem : baselineSharingGroup)
      {
        auto &flexItem = const_cast<RenderBox &>(*constFlexItem);
        auto position = AlignmentForFlexItem(flexItem);
        krys_debug_assert(position == ItemPosition::Baseline || position == ItemPosition::LastBaseline);
        auto offset = AlignmentOffset(AvailableAlignmentSpaceForFlexItem(lineCrossAxisExtent, flexItem),
                                      position, MarginBoxAscentForFlexItem(flexItem),
                                      baselineSharingGroup.MaxAscent(), containerHasWrapReverse);
        AdjustAlignmentForFlexItem(flexItem, offset);

        if (shouldAdjustItemTowardsCrossAxisEnd(
              flexItemWritingModeForBaselineAlignment(flexItem).BlockDirection(), position))
        {
          minMarginAfterBaseline =
            std::min(minMarginAfterBaseline,
                     AvailableAlignmentSpaceForFlexItem(lineCrossAxisExtent, flexItem) - offset);
        }
      }
      // css-align-3 9.3 part 3:
      // Position the aligned baseline-sharing group within the alignment container according to its
      // fallback alignment. The fallback alignment of a baseline-sharing group is the fallback alignment
      // of its items as resolved to physical directions.
      if (minMarginAfterBaseline)
      {
        for (const auto *constFlexItem : baselineSharingGroup)
        {
          auto &flexItem = const_cast<RenderBox &>(*constFlexItem);
          if (shouldAdjustItemTowardsCrossAxisEnd(
                flexItemWritingModeForBaselineAlignment(flexItem).BlockDirection(),
                AlignmentForFlexItem(flexItem))
              && !HasAutoMarginsInCrossAxis(flexItem))
          {
            AdjustAlignmentForFlexItem(flexItem, minMarginAfterBaseline);
          }
        }
      }
    }
  }

  void RenderFlexibleBox::ApplyStretchAlignmentToFlexItem(RenderBox &flexItem,
                                                          LayoutUnit lineCrossAxisExtent) noexcept
  {
    SetForScope flexLayoutScope(_afterCrossAxisItemSizing, true);
    if (MainAxisIsFlexItemInlineAxis(flexItem) && flexItem.Style().LogicalHeight().IsAuto())
    {
      LayoutUnit stretchedLogicalHeight =
        std::max(flexItem.BorderAndPaddingLogicalHeight(),
                 lineCrossAxisExtent - CrossAxisMarginExtentForFlexItem(flexItem));
      krys_debug_assert(!flexItem.NeedsLayout());
      LayoutUnit desiredLogicalHeight = flexItem.ConstrainLogicalHeightByMinMax(
        stretchedLogicalHeight, CachedFlexItemIntrinsicContentLogicalHeight(flexItem));

      // FIXME: Can avoid laying out here in some cases. See https://webkit.org/b/87905.
      bool flexItemNeedsRelayout = desiredLogicalHeight != flexItem.LogicalHeight();
      if (auto *block = DynamicDowncast<RenderBlock>(flexItem);
          block && block->HasPercentHeightDescendants() && _relaidOutFlexItems.contains(&flexItem))
      {
        // Have to force another relayout even though the child is sized
        // correctly, because its descendants are not sized correctly yet. Our
        // previous layout of the child was done without an override height set.
        // So, redo it here.
        flexItemNeedsRelayout = true;
      }
      if (flexItemNeedsRelayout || !flexItem.OverridingBorderBoxLogicalHeight())
      {
        flexItem.SetOverridingBorderBoxLogicalHeight(desiredLogicalHeight);
      }
      if (flexItemNeedsRelayout)
      {
        SetForScope resetFlexItemLogicalHeight(_shouldResetFlexItemLogicalHeightBeforeLayout, true);
        // We cache the child's intrinsic content logical height to avoid it being
        // reset to the stretched height.
        // FIXME: This is fragile. RenderBoxes should be smart enough to
        // determine their intrinsic content logical height correctly even when
        // there's an overrideHeight.
        LayoutUnit flexItemIntrinsicContentLogicalHeight =
          CachedFlexItemIntrinsicContentLogicalHeight(flexItem);
        flexItem.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);

        // Don't use layoutChildIfNeeded to avoid setting cross axis cached size twice.
        flexItem.LayoutIfNeeded();

        SetCachedFlexItemIntrinsicContentLogicalHeight(flexItem, flexItemIntrinsicContentLogicalHeight);
      }
    }
    else if (!MainAxisIsFlexItemInlineAxis(flexItem) && flexItem.Style().LogicalWidth().IsAuto())
    {
      LayoutUnit flexItemWidth =
        std::max(0_lu, lineCrossAxisExtent - CrossAxisMarginExtentForFlexItem(flexItem));
      flexItemWidth = flexItem.ConstrainLogicalWidthByMinMax(flexItemWidth, CrossAxisContentExtent(), *this);

      if (flexItemWidth != flexItem.LogicalWidth())
      {
        flexItem.SetOverridingBorderBoxLogicalWidth(flexItemWidth);
        flexItem.SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
        flexItem.LayoutIfNeeded();
      }
    }
  }

  void RenderFlexibleBox::FlipForRightToLeftColumn(const FlexLineStates &lineStates) noexcept
  {
    if (GetWritingMode().IsLogicalLeftInlineStart() || !IsColumnFlow())
    {
      return;
    }

    LayoutUnit crossExtent = CrossAxisExtent();
    for (auto &lineState : lineStates)
    {
      for (auto &flexLayoutItem : lineState.FlexLayoutItems)
      {
        auto &flexItem = *flexLayoutItem.Renderer;
        krys_debug_assert(!flexItem.IsOutOfFlowPositioned());

        LayoutPoint location = FlowAwareLocationForFlexItem(flexItem);
        // For vertical flows, setFlowAwareLocationForFlexItem will transpose x and
        // y, so using the y axis for a column cross axis extent is correct.
        location.SetY(crossExtent - CrossAxisExtentForFlexItem(flexItem) - location.Y());
        if (!IsHorizontalWritingMode())
        {
          location.Move(LayoutSize(0_lu, LayoutUnit(-HorizontalScrollbarHeight())));
        }
        SetFlowAwareLocationForFlexItem(flexItem, location);
      }
    }
  }

  void RenderFlexibleBox::FlipForWrapReverse(const FlexLineStates &lineStates,
                                             LayoutUnit crossAxisStartEdge) noexcept
  {
    LayoutUnit contentExtent = CrossAxisContentExtent();
    for (auto &lineState : lineStates)
    {
      for (auto &flexLayoutItem : lineState.FlexLayoutItems)
      {
        LayoutUnit lineCrossAxisExtent = lineState.CrossAxisExtent;
        LayoutUnit originalOffset = lineState.CrossAxisOffset - crossAxisStartEdge;
        LayoutUnit newOffset = contentExtent - originalOffset - lineCrossAxisExtent;
        AdjustAlignmentForFlexItem(*flexLayoutItem.Renderer, newOffset - originalOffset);
      }
    }
  }

  Maybe<TextDirection> RenderFlexibleBox::LeftRightAxisDirectionFromStyle(const RenderStyle &style) noexcept
  {
    if (!style.IsColumnFlexDirection()) // Prioritize text direction.
    {
      return style.GetWritingMode().BidiDirection();
    }

    if (style.GetWritingMode().IsVertical()) // Fall back to block direction if possible.
    {
      return style.GetWritingMode().IsBlockLeftToRight() ? TextDirection::LTR : TextDirection::RTL;
    }

    return null;
  }

  LayoutOptionalOutsets RenderFlexibleBox::AllowedLayoutOverflow() const noexcept
  {
    LayoutOptionalOutsets allowance = RenderBox::AllowedLayoutOverflow();

    bool isColumnar = Style().IsColumnFlexDirection();
    if (IsHorizontalWritingMode())
    {
      allowance.Top() = isColumnar ? _justifyContentStartOverflow : _alignContentStartOverflow;
      if (GetWritingMode().IsInlineLeftToRight())
      {
        allowance.Left() = isColumnar ? _alignContentStartOverflow : _justifyContentStartOverflow;
      }
      else
      {
        allowance.Right() = isColumnar ? _alignContentStartOverflow : _justifyContentStartOverflow;
      }
    }
    else
    {
      allowance.Left() = isColumnar ? _justifyContentStartOverflow : _alignContentStartOverflow;
      if (GetWritingMode().IsInlineTopToBottom())
      {
        allowance.Top() = isColumnar ? _alignContentStartOverflow : _justifyContentStartOverflow;
      }
      else
      {
        allowance.Bottom() = isColumnar ? _alignContentStartOverflow : _justifyContentStartOverflow;
      }
    }

    return allowance;
  }

  LayoutUnit RenderFlexibleBox::ComputeGap(GapType gapType) const noexcept
  {
    // row-gap is used for gaps between flex items in column flows or for gaps between lines in row flows.
    bool usesRowGap = (gapType == GapType::BetweenItems) == IsColumnFlow();
    auto &gap = usesRowGap ? Style().RowGap() : Style().ColumnGap();
    if (gap.IsNormal()) KRYS_LIKELY
    {
      return {};
    }

    auto availableSize =
      usesRowGap ? AvailableLogicalHeightForPercentageComputation().value_or(0_lu) : ContentBoxLogicalWidth();
    return style::EvaluateMinimum<LayoutUnit>(gap, availableSize, style::ZoomNeeded {});
  }

  bool RenderFlexibleBox::LayoutUsingFlexFormattingContext() noexcept
  {
    if (_hasFlexFormattingContextLayout && !*_hasFlexFormattingContextLayout)
    {
      // FIXME: Avoid continous content checking on (potentially) unsupported content. This ensures no pref
      // impact on cases like resize etc. Remove when canUseForFlexLayout becomes less expensive.
      return false;
    }

    _hasFlexFormattingContextLayout = layout_integration::FlexLayout::CanUseFor(*this);
    if (!*_hasFlexFormattingContextLayout)
    {
      return false;
    }

    auto flexLayout = layout_integration::FlexLayout {*this};
    flexLayout.UpdateFormattingContextGeometries();

    flexLayout.Layout();
    SetLogicalHeight(
      std::max(LogicalHeight(), BorderAndPaddingLogicalHeight() + flexLayout.ContentBoxLogicalHeight()));
    UpdateLogicalHeight();
    return true;
  }

  const RenderBox *RenderFlexibleBox::FirstBaselineCandidateOnLine(OrderIterator flexItemIterator,
                                                                   size_t numberOfItemsOnLine) const noexcept
  {
    // Note that "first" here means in iterator order and not logical flex order (caller can pass in reversed
    // order).
    size_t index = 0;
    const RenderBox *baselineFlexItem = nullptr;
    for (auto *flexItem = flexItemIterator.First(); flexItem; flexItem = flexItemIterator.Next())
    {
      if (flexItemIterator.ShouldSkipChild(*flexItem))
      {
        continue;
      }
      auto flexItemPosition = AlignmentForFlexItem(*flexItem);
      if ((flexItemPosition == ItemPosition::Baseline || flexItemPosition == ItemPosition::LastBaseline)
          && MainAxisIsFlexItemInlineAxis(*flexItem) && !HasAutoMarginsInCrossAxis(*flexItem))
      {
        return flexItem;
      }
      if (!baselineFlexItem)
      {
        baselineFlexItem = flexItem;
      }
      if (++index == numberOfItemsOnLine)
      {
        return baselineFlexItem;
      }
    }
    return nullptr;
  }

  const RenderBox *RenderFlexibleBox::LastBaselineCandidateOnLine(OrderIterator flexItemIterator,
                                                                  size_t numberOfItemsOnLine) const noexcept
  {
    // Note that "last" here means in iterator order and not logical flex order (caller can pass in reversed
    // order).
    size_t index = 0;
    RenderBox *baselineFlexItem = nullptr;
    for (auto *flexItem = flexItemIterator.First(); flexItem; flexItem = flexItemIterator.Next())
    {
      if (flexItemIterator.ShouldSkipChild(*flexItem))
      {
        continue;
      }
      auto flexItemPosition = AlignmentForFlexItem(*flexItem);
      if ((flexItemPosition == ItemPosition::Baseline || flexItemPosition == ItemPosition::LastBaseline)
          && MainAxisIsFlexItemInlineAxis(*flexItem) && !HasAutoMarginsInCrossAxis(*flexItem))
      {
        baselineFlexItem = flexItem;
      }
      if (++index == numberOfItemsOnLine)
      {
        return baselineFlexItem ? baselineFlexItem : flexItem;
      }
    }
    return nullptr;
  }

  const RenderBox *RenderFlexibleBox::FlexItemForFirstBaseline() const noexcept
  {
    // Looking for baseline flex candidate on visually first line.
    auto useLastLine = IsWrapReverse();
    auto useLastItem = Style().GetFlexDirection() == FlexDirection::RowReverse
                       || Style().GetFlexDirection() == FlexDirection::ColumnReverse;

    if (!useLastLine)
    {
      if (!useLastItem)
      {
        // Logically (and visually) first item on logically (and visually) first line.
        return FirstBaselineCandidateOnLine(_orderIterator, _numberOfFlexItemsOnFirstLine);
      }
      // Logically last (but visually first) item on logically (and visually) first line.
      return LastBaselineCandidateOnLine(_orderIterator, _numberOfFlexItemsOnFirstLine);
    }

    if (!useLastItem)
    {
      // Logically (and visually) first item on logically last (but visually first) line.
      return LastBaselineCandidateOnLine(_orderIterator.Reverse(), _numberOfFlexItemsOnLastLine);
    }
    // Logically last (but visually first) item on logically last (but visually first) line.
    return FirstBaselineCandidateOnLine(_orderIterator.Reverse(), _numberOfFlexItemsOnLastLine);
  }

  const RenderBox *RenderFlexibleBox::FlexItemForLastBaseline() const noexcept
  {
    // Looking for baseline flex candidate on visually last line.
    auto useLastLine = IsWrapReverse();
    auto useLastItem = Style().GetFlexDirection() == FlexDirection::RowReverse
                       || Style().GetFlexDirection() == FlexDirection::ColumnReverse;

    if (!useLastLine)
    {
      if (!useLastItem)
      {
        // Logically (and visually) last item on logically (and visually) last line.
        return FirstBaselineCandidateOnLine(_orderIterator.Reverse(), _numberOfFlexItemsOnLastLine);
      }
      // Logically first (but visually last) item  on logically (and visually) last line.
      return LastBaselineCandidateOnLine(_orderIterator.Reverse(), _numberOfFlexItemsOnLastLine);
    }

    if (!useLastItem)
    {
      // Logically (and visually) last item on logically first (but visually last) line.
      return LastBaselineCandidateOnLine(_orderIterator, _numberOfFlexItemsOnFirstLine);
    }
    // Logically first (but visually last) item on logically last (but visually first) line.
    return FirstBaselineCandidateOnLine(_orderIterator, _numberOfFlexItemsOnFirstLine);
  }
  void RenderFlexibleBox::PaintChildren(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
                                        PaintInfo &paintInfoForFlexItem) noexcept
  {
    for (RenderBox *flexItem = _orderIterator.First(); flexItem; flexItem = _orderIterator.Next())
    {
      if (!PaintChild(*flexItem, paintInfo, paintOffset, paintInfoForFlexItem, PaintAsInlineBlock))
      {
        return;
      }
    }
  }

  bool RenderFlexibleBox::HitTestChildren(const HitTestRequest &request, HitTestResult &result,
                                          const HitTestLocation &locationInContainer,
                                          const LayoutPoint &adjustedLocation,
                                          HitTestAction hitTestAction) noexcept
  {
    if (hitTestAction != HitTestForeground)
    {
      return false;
    }

    LayoutPoint scrolledOffset = HasNonVisibleOverflow()
                                   ? adjustedLocation - ToLayoutSize(LayoutPoint(GetScrollPosition()))
                                   : adjustedLocation;

    // If collecting the children in reverse order is bad for performance, this list could be determined at
    // layout time.
    List<RenderBox *> reversedOrderIteratorForHitTesting;
    for (auto *flexItem = _orderIterator.First(); flexItem; flexItem = _orderIterator.Next())
    {
      if (flexItem->IsOutOfFlowPositioned())
      {
        continue;
      }
      reversedOrderIteratorForHitTesting.push_back(flexItem);
    }
    std::ranges::reverse(reversedOrderIteratorForHitTesting);

    for (auto *flexItem : reversedOrderIteratorForHitTesting)
    {
      if (flexItem->HasSelfPaintingLayer())
      {
        continue;
      }
      auto location = FlipForWritingModeForChild(*flexItem, scrolledOffset);
      if (flexItem->HitTest(request, result, locationInContainer, location))
      {
        UpdateHitTestResult(
          result, FlipForWritingMode(ToLayoutPoint(locationInContainer.Point() - adjustedLocation)));
        return true;
      }
    }

    return false;
  }
}
