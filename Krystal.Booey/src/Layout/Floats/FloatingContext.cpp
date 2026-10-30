#include "Krystal.Booey/Layout/Floats/FloatingContext.hpp"
#include "Krystal.Booey/Layout/Floats/FloatAvoider.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutContainingBlockChainIterator.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>
#include <ranges>

namespace krys::boo::layout
{
  // Finding the top/left position for a new floating(F)
  //  ____  ____  _____               _______
  // |    || L2 ||     | <-----1---->|       |
  // |    ||____||  L3 |             |   R1  |
  // | L1 |      |_____|             |       |
  // |____| <-------------2--------->|       |
  //                                 |       |
  //                                 |_______|
  //
  // 1. Compute the initial vertical position for (F) -> (1)
  // 2. Find the corresponding floating pair (L3-R1)
  // 3. Align (F) horizontally with (L3-R1) depending whether (F) is left/right positioned
  // 4. Intersect (F) with (L3-R1)
  // 5. If (F) does not fit, find the next floating pair (L1-R1)
  // 6. Repeat until either (F) fits/no more floats.
  // Note that all coordinates are in the coordinate system of the formatting root.
  // The formatting root here is always the one that establishes the floating context (see inherited floating
  // context). (It simply means that the float box's formatting root is not necessarily the same as the
  // FormattingContext's root.)

  namespace
  {
    class Iterator;

    class FloatPair
    {
      friend class Iterator;

    public:
      struct InlineStartEndIndex
      {
        KRYS_NODISCARD bool IsEmpty() const noexcept
        {
          return !InlineStart && !InlineEnd;
        }

        Maybe<size_t> InlineStart;
        Maybe<size_t> InlineEnd;
      };

      struct InlineAxisConstraints
      {
        Maybe<PositionInContextRoot> Start;
        Maybe<PositionInContextRoot> End;
      };

    private:
      const PlacedFloats::List &_floats;
      InlineStartEndIndex _floatPair;
      PositionInContextRoot _highestBlockAxisPosition;

    public:
      KRYS_NODISCARD bool IsEmpty() const noexcept
      {
        return _floatPair.IsEmpty();
      }

      KRYS_NODISCARD const PlacedFloats::Item *InlineStart() const noexcept;
      KRYS_NODISCARD const PlacedFloats::Item *InlineEnd() const noexcept;

      KRYS_NODISCARD bool Intersects(const FloatAvoider &floatAvoider) const noexcept;
      KRYS_NODISCARD bool
        Intersects(BoxGeometry::HorizontalEdges containingBlockContentBoxEdges) const noexcept;
      KRYS_NODISCARD bool ContainsFloatFromFormattingContext() const noexcept;

      KRYS_NODISCARD PositionInContextRoot HighestBlockAxisPosition() const noexcept
      {
        return _highestBlockAxisPosition;
      }
      KRYS_NODISCARD PositionInContextRoot LowestBlockAxisPosition() const noexcept;

      KRYS_NODISCARD InlineAxisConstraints GetInlineAxisConstraints() const noexcept;

      KRYS_NODISCARD InlineStartEndIndex operator*() const noexcept
      {
        return _floatPair;
      }

      KRYS_NODISCARD bool operator==(const FloatPair &other) const noexcept
      {
        return _floatPair.InlineStart == other._floatPair.InlineStart
               && _floatPair.InlineEnd == other._floatPair.InlineEnd;
      }

    private:
      explicit FloatPair(const PlacedFloats::List &floats) noexcept : _floats(floats)
      {
      }
    };

    class Iterator
    {
    private:
      const PlacedFloats::List &_floats;
      FloatPair _current;

    public:
      Iterator(const PlacedFloats::List &floats, Maybe<PositionInContextRoot> blockStart) noexcept;

      KRYS_NODISCARD const FloatPair &operator*() const noexcept
      {
        return _current;
      }

      Iterator &operator++() noexcept;

      KRYS_NODISCARD bool operator==(const Iterator &other) const noexcept
      {
        return _current == other._current;
      }

    private:
      void Set(PositionInContextRoot blockAxisPosition) noexcept;
    };

    KRYS_NODISCARD Iterator Begin(const PlacedFloats::List &floats,
                                  PositionInContextRoot initialBlockStart) noexcept
    {
      // Start with the inner-most floating pair for the initial vertical position.
      return Iterator(floats, initialBlockStart);
    }

    KRYS_NODISCARD Iterator End(const PlacedFloats::List &floats) noexcept
    {
      return Iterator(floats, null);
    }

#if KRYS_DEBUG
    KRYS_NODISCARD bool AreFloatsHorizontallySorted(const PlacedFloats &placedFloats) noexcept
    {
      auto &floats = placedFloats.GetList();
      auto inlineEndEdgeOfInlineStartFloats = LayoutUnit::Min();
      auto inlineStartEdgeOfInlineEndFloats = LayoutUnit::Max();
      auto lowestBlockAxisPositionAtInlineStart = Maybe<LayoutUnit> {};
      auto lowestBlockAxisPositionAtInlineEnd = Maybe<LayoutUnit> {};

      for (auto &floatItem : floats)
      {
        if (floatItem.IsStartPositioned())
        {
          auto inlineEndEdge = floatItem.AbsoluteRectWithMargin().Right();
          if (inlineEndEdge < inlineEndEdgeOfInlineStartFloats)
          {
            if (lowestBlockAxisPositionAtInlineStart
                && floatItem.AbsoluteRectWithMargin().Top() < *lowestBlockAxisPositionAtInlineStart)
            {
              return false;
            }
          }
          lowestBlockAxisPositionAtInlineStart = floatItem.AbsoluteRectWithMargin().Bottom();
          inlineEndEdgeOfInlineStartFloats = inlineEndEdge;
        }
        else
        {
          auto inlineStarEdge = floatItem.AbsoluteRectWithMargin().Left();
          if (inlineStarEdge > inlineStartEdgeOfInlineEndFloats)
          {
            if (lowestBlockAxisPositionAtInlineEnd
                && floatItem.AbsoluteRectWithMargin().Top() < *lowestBlockAxisPositionAtInlineEnd)
            {
              return false;
            }
          }
          lowestBlockAxisPositionAtInlineEnd = floatItem.AbsoluteRectWithMargin().Bottom();
          inlineStartEdgeOfInlineEndFloats = inlineStarEdge;
        }
      }
      return true;
    }
#endif

    FloatPair::InlineStartEndIndex
      FindAvailablePosition(FloatAvoider &floatAvoider, const PlacedFloats::List &floats,
                            BoxGeometry::HorizontalEdges containingBlockContentBoxEdges) noexcept
    {
      auto lowestBlockAxisPosition = Maybe<PositionInContextRoot> {};
      auto innerMostInlineStartAndEnd = Maybe<FloatPair::InlineStartEndIndex> {};
      auto end = End(floats);
      for (auto iterator = Begin(floats, {floatAvoider.BlockStart()}); iterator != end; ++iterator)
      {
        krys_debug_assert(!(*iterator).IsEmpty());
        auto &inlineStartEndFloatPair = *iterator;
        innerMostInlineStartAndEnd = innerMostInlineStartAndEnd.value_or(*inlineStartEndFloatPair);

        // Move the box horizontally so that it either
        // 1. aligns with the current floating pair (always constrained by containing block e.g. when current
        // float on this position is outside of containing block i.e. not intrusive).
        // 2. or with the containing block's content box if there's no float to align with at this vertical
        // position.
        auto inlineStartEndEdge = inlineStartEndFloatPair.GetInlineAxisConstraints();
        // Ensure that the float avoider
        // 1. avoids floats on both sides (with the exception of non-intrusive floats from other FCs)
        // 2. does not overflow its containing block if the horizontal position is constrained by other
        // floats (i.e. a float avoider may overflow its containing block just fine unless this overflow is
        // the result of getting it pushed by other floats on this vertical position -out of available space)
        // 3. Move to the next floating pair if this vertical position is over-constrained.
        if (auto inlineAxisConstraints =
              floatAvoider.IsStartAligned() ? inlineStartEndEdge.Start : inlineStartEndEdge.End)
        {
          floatAvoider.SetInlineStart(*inlineAxisConstraints);
        }
        else
        {
          floatAvoider.ResetInlineStart();
        }
        floatAvoider.SetBlockStart(inlineStartEndFloatPair.HighestBlockAxisPosition());

        if (!inlineStartEndFloatPair.Intersects(floatAvoider) && !floatAvoider.OverflowsContainingBlock())
        {
          return *innerMostInlineStartAndEnd;
        }

        // Is this float pair is outside of our containing block's content box? In some cases we _may_ overlap
        // them.
        if (!inlineStartEndFloatPair.Intersects(containingBlockContentBoxEdges)
            && !inlineStartEndFloatPair.ContainsFloatFromFormattingContext())
        {
          // Surprisingly floats do overlap each other on the non-floating side (e.g. float: left may overlap
          // a float: right) when they are not considered intrusive (i.e. they are outside of our containing
          // block's content box) and coming from outside of the formatting context.
          return *innerMostInlineStartAndEnd;
        }

        lowestBlockAxisPosition = inlineStartEndFloatPair.LowestBlockAxisPosition();
        // Move to the next floating pair.
      }

      // The candidate box is already below of all the floats.
      if (!lowestBlockAxisPosition)
      {
        return {};
      }

      // Passed all the floats and still does not fit? Push it below the last float.
      floatAvoider.SetBlockStart(*lowestBlockAxisPosition);
      floatAvoider.ResetInlineStart();
      krys_debug_assert(innerMostInlineStartAndEnd);
      return *innerMostInlineStartAndEnd;
    }
  }

  struct FloatingContext::AbsoluteCoordinateValuesForFloatAvoider
  {
    LayoutPoint TopLeft;
    LayoutPoint ContainingBlockTopLeft;
    BoxGeometry::HorizontalEdges ContainingBlockContentBox;
  };

  FloatingContext::FloatingContext(const ElementBox &formattingContextRoot, const LayoutState &layoutState,
                                   const PlacedFloats &placedFloats) noexcept
      : _formattingContextRoot(ShareCheckedRef(formattingContextRoot)),
        _layoutState(ShareCheckedRef(layoutState)), _placedFloats(placedFloats)
  {
  }

  LayoutPoint
    FloatingContext::PositionForFloat(const Box &layoutBox, const BoxGeometry &boxGeometry,
                                      const HorizontalConstraints &horizontalConstraints) const noexcept
  {
    krys_debug_assert(layoutBox.IsFloatingPositioned());
    krys_debug_assert(AreFloatsHorizontallySorted(_placedFloats));

    auto borderBoxTopLeft = BoxGeometry::BorderBoxTopLeft(boxGeometry);
    if (IsEmpty())
    {
      auto alignWithContainingBlock = [&]() -> Position
      {
        // If there is no floating to align with, push the box to the left/right edge of its containing
        // block's content box.
        if (IsFloatingCandidateStartPositionedInBlockFormattingContext(layoutBox))
        {
          return {horizontalConstraints.LogicalLeft + boxGeometry.MarginStart()};
        }
        return {horizontalConstraints.LogicalRight() - boxGeometry.MarginEnd()
                - boxGeometry.BorderBoxWidth()};
      };

      // No float box on the context yet -> align it with the containing block's left/right edge.
      return {alignWithContainingBlock(), borderBoxTopLeft.Y()};
    }

    // Find the top most position where the float box fits.
    krys_debug_assert(!IsEmpty());

    auto clearPosition = [&]() -> Maybe<LayoutUnit>
    {
      if (!layoutBox.HasFloatClear())
      {
        return null;
      }

      // The vertical position candidate needs to clear the existing floats in this context.
      switch (ClearInBlockFormattingContext(layoutBox))
      {
        case boo::Clear::Left:  return GetPlacedFloats().LowestPositionOnBlockAxis(boo::Clear::InlineStart);
        case boo::Clear::Right: return GetPlacedFloats().LowestPositionOnBlockAxis(boo::Clear::InlineEnd);
        case boo::Clear::Both:  return GetPlacedFloats().LowestPositionOnBlockAxis();
        default:                krys_debug_assert(false);
      }
      return null;
    };

    auto absoluteCoordinates = AbsoluteCoordinates(layoutBox, borderBoxTopLeft);
    auto absoluteTopLeft = absoluteCoordinates.TopLeft;
    auto blockStartCandidate = absoluteTopLeft.Y();

    // Incoming float cannot be placed higher than existing floats (margin box of the last float).
    // Take the static position (where the box would go if it wasn't floating) and adjust it with the last
    // float.
    auto lastFloatAbsoluteTop = GetPlacedFloats().Last()->AbsoluteRectWithMargin().Top();
    auto lastOrClearedFloatPosition =
      std::max(clearPosition().value_or(lastFloatAbsoluteTop), lastFloatAbsoluteTop);
    if (blockStartCandidate - boxGeometry.MarginBefore() < lastOrClearedFloatPosition)
    {
      blockStartCandidate = lastOrClearedFloatPosition + boxGeometry.MarginBefore();
    }
    absoluteTopLeft.SetY(blockStartCandidate);

    auto margins = BoxGeometry::Edges {{boxGeometry.MarginStart(), boxGeometry.MarginEnd()},
                                       {boxGeometry.MarginBefore(), boxGeometry.MarginAfter()}};
    auto floatBox = FloatAvoider {absoluteTopLeft,
                                  boxGeometry.BorderBoxWidth(),
                                  margins,
                                  absoluteCoordinates.ContainingBlockContentBox,
                                  true,
                                  IsFloatingCandidateStartPositionedInBlockFormattingContext(layoutBox)};
    (void)FindAvailablePosition(floatBox, GetPlacedFloats().GetList(),
                                absoluteCoordinates.ContainingBlockContentBox);

    // Convert box coordinates from formatting root back to containing block.
    auto containingBlockTopLeft = absoluteCoordinates.ContainingBlockTopLeft;
    return {floatBox.InlineStart() + margins.Horizontal.Start - containingBlockTopLeft.X(),
            floatBox.BlockStart() + margins.Vertical.Before - containingBlockTopLeft.Y()};
  }

  LayoutPoint
    FloatingContext::PositionForNonFloatingFloatAvoider(const Box &layoutBox,
                                                        const BoxGeometry &boxGeometry) const noexcept
  {
    krys_debug_assert(layoutBox.EstablishesBlockFormattingContext());
    krys_debug_assert(!layoutBox.IsFloatingPositioned());
    krys_debug_assert(!layoutBox.HasFloatClear());
    krys_debug_assert(AreFloatsHorizontallySorted(_placedFloats));

    auto borderBoxTopLeft = BoxGeometry::BorderBoxTopLeft(boxGeometry);
    if (IsEmpty())
    {
      return borderBoxTopLeft;
    }

    auto absoluteCoordinates = AbsoluteCoordinates(layoutBox, borderBoxTopLeft);
    auto margins = BoxGeometry::Edges {{boxGeometry.MarginStart(), boxGeometry.MarginEnd()},
                                       {boxGeometry.MarginBefore(), boxGeometry.MarginAfter()}};
    auto floatAvoider = FloatAvoider {absoluteCoordinates.TopLeft,
                                      boxGeometry.BorderBoxWidth(),
                                      margins,
                                      absoluteCoordinates.ContainingBlockContentBox,
                                      false,
                                      layoutBox.GetWritingMode().IsBidiLTR()};
    FindPositionForFormattingContextRoot(floatAvoider, absoluteCoordinates.ContainingBlockContentBox);
    auto containingBlockTopLeft = absoluteCoordinates.ContainingBlockTopLeft;
    return {floatAvoider.InlineStart() - containingBlockTopLeft.X(),
            floatAvoider.BlockStart() - containingBlockTopLeft.Y()};
  }

  Maybe<FloatingContext::BlockAxisPositionWithClearance>
    FloatingContext::GetBlockAxisPositionWithClearance(const Box &layoutBox,
                                                       const BoxGeometry &boxGeometry) const noexcept
  {
    krys_debug_assert(layoutBox.HasFloatClear());
    krys_debug_assert(AreFloatsHorizontallySorted(_placedFloats));

    if (IsEmpty())
    {
      return null;
    }

    auto lowestPositionOnBlockAxis =
      [&](Maybe<LayoutUnit> blockAxisPosition) -> Maybe<BlockAxisPositionWithClearance>
    {
      if (!blockAxisPosition)
      {
        return null;
      }

      // 9.5.2 Controlling flow next to floats: the 'clear' property
      // Then the amount of clearance is set to the greater of:
      //
      // 1. The amount necessary to place the border edge of the block even with the bottom outer edge of the
      // lowest float that is to be cleared.
      // 2. The amount necessary to place the top border edge of the block at its hypothetical position.
      auto logicalTopRelativeToBlockFormattingContextRoot =
        MapTopLeftToBlockFormattingContextRoot(layoutBox, BoxGeometry::BorderBoxTopLeft(boxGeometry)).Y();
      auto clearance = *blockAxisPosition - logicalTopRelativeToBlockFormattingContextRoot;
      if (clearance <= 0)
      {
        return null;
      }

      if (layoutBox.IsBlockLevelBox())
      {
        // Clearance inhibits margin collapsing in block formatting context.
        // FIXME: This needs to go to BFC.
        krys_debug_assert_msg(false, "Not implemented yet");
      }

      // Now adjust the box's position with the clearance.
      logicalTopRelativeToBlockFormattingContextRoot += clearance;
      krys_debug_assert(*blockAxisPosition == logicalTopRelativeToBlockFormattingContextRoot);

      // The return vertical position needs to be in the containing block's coordinate system.
      auto &containingBlock = FormattingContext::ContainingBlock(layoutBox);
      if (&containingBlock == &GetPlacedFloats().BlockFormattingContextRoot())
      {
        return BlockAxisPositionWithClearance {logicalTopRelativeToBlockFormattingContextRoot, clearance};
      }

      auto containingBlockTopLeft =
        BoxGeometry::BorderBoxTopLeft(ContainingBlockGeometries().GeometryForBox(containingBlock));
      auto containingBlockRootRelativeTop =
        MapTopLeftToBlockFormattingContextRoot(containingBlock, containingBlockTopLeft).Y();
      return BlockAxisPositionWithClearance {
        logicalTopRelativeToBlockFormattingContextRoot - containingBlockRootRelativeTop, clearance};
    };

    auto clear = ClearInBlockFormattingContext(layoutBox);
    if (clear == boo::Clear::Left)
    {
      return lowestPositionOnBlockAxis(GetPlacedFloats().LowestPositionOnBlockAxis(boo::Clear::InlineStart));
    }
    if (clear == boo::Clear::Right)
    {
      return lowestPositionOnBlockAxis(GetPlacedFloats().LowestPositionOnBlockAxis(boo::Clear::InlineEnd));
    }
    if (clear == boo::Clear::Both)
    {
      return lowestPositionOnBlockAxis(GetPlacedFloats().LowestPositionOnBlockAxis());
    }
    krys_debug_assert(false);
    return null;
  }

  FloatingContext::Constraints
    FloatingContext::GetConstraints(LayoutUnit candidateTop, LayoutUnit candidateBottom,
                                    MayBeAboveLastFloat mayBeAboveLastFloat) const noexcept
  {
    if (IsEmpty())
    {
      return {};
    }

    // 1. Convert vertical position if this floating context is inherited.
    // 2. Find the inner left/right floats at candidateTop/candidateBottom. Note when MayBeAboveLastFloat is
    // 'no', we can just stop at the inner most (last) float (block vs. inline case).
    // 3. Convert left/right positions back to formattingContextRoot's coordinate system.
    auto &placedFloats = GetPlacedFloats();
    auto coordinateMappingIsRequired = &placedFloats.BlockFormattingContextRoot() != &Root();
    auto adjustedCandidateTop = candidateTop;
    LayoutSize adjustingDelta;
    if (coordinateMappingIsRequired)
    {
      auto adjustedCandidatePosition =
        MapPointFromFloatingContextRootToBlockFormattingContextRoot({0_lu, candidateTop});
      adjustedCandidateTop = adjustedCandidatePosition.Y;
      adjustingDelta = {adjustedCandidatePosition.X, adjustedCandidateTop - candidateTop};
    }
    auto adjustedCandidateBottom = adjustedCandidateTop + (candidateBottom - candidateTop);
    auto candidateHeight = adjustedCandidateBottom - adjustedCandidateTop;

    auto contains = [&](const Rect &floatBoxRect)
    {
      if (floatBoxRect.IsEmpty())
      {
        return false;
      }
      if (!candidateHeight)
      {
        return floatBoxRect.Top() <= adjustedCandidateTop && floatBoxRect.Bottom() > adjustedCandidateTop;
      }
      return floatBoxRect.Top() < adjustedCandidateBottom && floatBoxRect.Bottom() > adjustedCandidateTop;
    };

    auto computeFloatEdgeAndBottom =
      [&](const PlacedFloats::Item &floatItem) -> Maybe<Pair<LayoutUnit, LayoutUnit>>
    {
      auto marginRect = floatItem.AbsoluteRectWithMargin();
      if (!contains(marginRect))
      {
        return null;
      }

      if (auto *shape = floatItem.Shape())
      {
        // Shapes are relative to the border box.
        auto borderRect = floatItem.AbsoluteBorderBoxRect();
        auto positionInShape = adjustedCandidateTop - borderRect.Top();

        if (!shape->LineOverlapsShapeMarginBounds(positionInShape, candidateHeight))
        {
          return null;
        }

        // PolygonShape gets confused when passing in 0px height interval at vertices.
        auto segment = shape->GetExcludedInterval(positionInShape, std::max(candidateHeight, 1_lu));
        if (!segment.IsValid)
        {
          return null;
        }

        // Bottom is used to decide the next line top if nothing fits. With shape we'll just sample one pixel
        // down.
        // FIXME: This is potentially slow.
        auto bottom = adjustedCandidateTop + 1_lu;
        if (floatItem.IsStartPositioned())
        {
          auto shapeRight = borderRect.Left() + LayoutUnit(segment.LogicalRight);
          // Shape can't extend beyond the margin box.
          return Pair<LayoutUnit, LayoutUnit> {std::min(shapeRight, marginRect.Right()), bottom};
        }
        auto shapeLeft = borderRect.Left() + LayoutUnit(segment.LogicalLeft);
        return Pair<LayoutUnit, LayoutUnit> {std::max(shapeLeft, marginRect.Left()), bottom};
      }

      auto edge = floatItem.IsStartPositioned() ? marginRect.Right() : marginRect.Left();
      return Pair<LayoutUnit, LayoutUnit> {edge, marginRect.Bottom()};
    };

    auto constraints = Constraints {};
    if (mayBeAboveLastFloat == MayBeAboveLastFloat::No)
    {
      for (auto &floatItem : placedFloats.GetList() | std::views::reverse)
      {
        if ((constraints.Start && floatItem.IsStartPositioned())
            || (constraints.End && !floatItem.IsStartPositioned()))
        {
          continue;
        }

        auto edgeAndBottom = computeFloatEdgeAndBottom(floatItem);
        if (!edgeAndBottom)
        {
          continue;
        }

        auto [edge, bottom] = *edgeAndBottom;
        if (floatItem.IsStartPositioned())
        {
          constraints.Start = PointInContextRoot {edge, bottom};
        }
        else
        {
          constraints.End = PointInContextRoot {edge, bottom};
        }

        if ((constraints.Start && constraints.End) || (constraints.Start && !placedFloats.HasEndPositioned())
            || (constraints.End && !placedFloats.HasStartPositioned()))
        {
          break;
        }
      }
    }
    else
    {
      for (auto &floatItem : placedFloats.GetList() | std::views::reverse)
      {
        auto edgeAndBottom = computeFloatEdgeAndBottom(floatItem);
        if (!edgeAndBottom)
        {
          continue;
        }

        auto [edge, bottom] = *edgeAndBottom;
        if (floatItem.IsStartPositioned())
        {
          if (!constraints.Start || constraints.Start->X < edge)
          {
            constraints.Start = PointInContextRoot {edge, bottom};
          }
        }
        else
        {
          if (!constraints.End || constraints.End->X > edge)
          {
            constraints.End = PointInContextRoot {edge, bottom};
          }
        }
        // FIXME: Bail out when floats are way above.
      }
    }

    if (coordinateMappingIsRequired)
    {
      if (constraints.Start)
      {
        constraints.Start->Move(-adjustingDelta);
      }
      if (constraints.End)
      {
        constraints.End->Move(-adjustingDelta);
      }
    }

    if (placedFloats.BlockFormattingContextRoot().Style().GetWritingMode().IsInlineOpposing(
          Root().GetWritingMode()))
    {
      // FIXME: Move it under coordinateMappingIsRequired when the integration codepath starts initiating the
      // floating state with the correct containing block (i.e. when the float comes from the parent BFC).
      // Flip to logical in inline direction.
      auto logicalConstraints = Constraints {};
      auto borderBoxWidth = ContainingBlockGeometries().GeometryForBox(Root()).BorderBoxWidth();
      if (constraints.Start)
      {
        logicalConstraints.End =
          PointInContextRoot {borderBoxWidth - constraints.Start->X, constraints.Start->Y};
      }
      if (constraints.End)
      {
        logicalConstraints.Start =
          PointInContextRoot {borderBoxWidth - constraints.End->X, constraints.End->Y};
      }
      constraints = logicalConstraints;
    }

    return constraints;
  }

  PlacedFloats::Item FloatingContext::MakeFloatItem(const Box &floatBox, const BoxGeometry &boxGeometry,
                                                    Maybe<size_t> line) const noexcept
  {
    auto borderBoxTopLeft = BoxGeometry::BorderBoxTopLeft(boxGeometry);
    auto absoluteBoxGeometry = BoxGeometry {boxGeometry};
    absoluteBoxGeometry.SetTopLeft(MapTopLeftToBlockFormattingContextRoot(floatBox, borderBoxTopLeft));
    auto position = IsFloatingCandidateStartPositionedInBlockFormattingContext(floatBox)
                      ? PlacedFloats::Item::Position::Start
                      : PlacedFloats::Item::Position::End;
    return {floatBox, position, absoluteBoxGeometry, borderBoxTopLeft, line};
  }

  const LayoutState &FloatingContext::ContainingBlockGeometries() const noexcept
  {
    return *_layoutState;
  }

  void FloatingContext::FindPositionForFormattingContextRoot(
    FloatAvoider &floatAvoider, BoxGeometry::HorizontalEdges containingBlockContentBoxEdges) const noexcept
  {
    // A non-floating formatting root's initial vertical position is its static position.
    // It means that such boxes can end up vertically placed in-between existing floats (which is
    // never the case for floats, since they cannot be placed above existing floats).
    //  ____  ____
    // |    || F1 |
    // | L1 | ----
    // |    |  ________
    //  ----  |   R1   |
    //         --------
    // Document order: 1. float: left (L1) 2. float: right (R1) 3. formatting root (F1)
    //
    // 1. Probe for available placement at initial position (note it runs a backward probing algorithm at a
    // specific vertical position)
    // 2. Check if there's any intersecting float below (forward search)
    // 3. Align the box with the intersected float and probe for placement again (#1).
    auto &floats = _placedFloats.GetList();
    while (true)
    {
      auto innerMostInlineStartAndEnd =
        FindAvailablePosition(floatAvoider, floats, containingBlockContentBoxEdges);
      if (innerMostInlineStartAndEnd.IsEmpty())
      {
        return;
      }

      auto overlappingFloatBox = [&floats](size_t startFloatIndex,
                                           const FloatAvoider &floatAvoider) -> const PlacedFloats::Item *
      {
        for (auto i = startFloatIndex; i < floats.size(); ++i)
        {
          auto &floatBox = floats[i];

          auto intersects = [&]
          {
            auto floatingRect = floatBox.AbsoluteRectWithMargin();
            if (floatAvoider.InlineStart() >= floatingRect.Right()
                || floatAvoider.InlineEnd() <= floatingRect.Left())
            {
              return false;
            }
            return floatAvoider.BlockStart() >= floatingRect.Top()
                   && floatAvoider.BlockStart() < floatingRect.Bottom();
          }();

          if (intersects)
          {
            return &floatBox;
          }
        }
        return nullptr;
      };

      auto startIndex = std::max(innerMostInlineStartAndEnd.InlineStart.value_or(0),
                                 innerMostInlineStartAndEnd.InlineEnd.value_or(0))
                        + 1;
      auto *intersectedFloatBox = overlappingFloatBox(startIndex, floatAvoider);
      if (!intersectedFloatBox)
      {
        return;
      }
      floatAvoider.SetBlockStart(intersectedFloatBox->AbsoluteRectWithMargin().Top());
    }
  }

  FloatingContext::AbsoluteCoordinateValuesForFloatAvoider
    FloatingContext::AbsoluteCoordinates(const Box &floatAvoider, LayoutPoint borderBoxTopLeft) const noexcept
  {
    auto &containingBlock = FormattingContext::ContainingBlock(floatAvoider);
    auto &containingBlockGeometry = ContainingBlockGeometries().GeometryForBox(containingBlock);

    auto absoluteTopLeft = MapTopLeftToBlockFormattingContextRoot(floatAvoider, borderBoxTopLeft);
    if (&containingBlock == &GetPlacedFloats().BlockFormattingContextRoot())
    {
      return {absoluteTopLeft,
              {},
              {containingBlockGeometry.ContentBoxLeft(), containingBlockGeometry.ContentBoxRight()}};
    }

    auto containingBlockAbsoluteTopLeft = MapTopLeftToBlockFormattingContextRoot(
      containingBlock, BoxGeometry::BorderBoxTopLeft(containingBlockGeometry));
    return {absoluteTopLeft,
            containingBlockAbsoluteTopLeft,
            {containingBlockAbsoluteTopLeft.X() + containingBlockGeometry.ContentBoxLeft(),
             containingBlockAbsoluteTopLeft.X() + containingBlockGeometry.ContentBoxRight()}};
  }

  LayoutPoint
    FloatingContext::MapTopLeftToBlockFormattingContextRoot(const Box &layoutBox,
                                                            LayoutPoint borderBoxTopLeft) const noexcept
  {
    krys_debug_assert(layoutBox.IsFloatingPositioned() || layoutBox.IsInFlow());
    auto &blockFormattingContextRoot = GetPlacedFloats().BlockFormattingContextRoot();
    for (auto &containingBlock : ContainingBlockChain(layoutBox, blockFormattingContextRoot))
    {
      borderBoxTopLeft.MoveBy(
        BoxGeometry::BorderBoxTopLeft(ContainingBlockGeometries().GeometryForBox(containingBlock)));
    }
    return borderBoxTopLeft;
  }

  Point FloatingContext::MapPointFromFloatingContextRootToBlockFormattingContextRoot(
    Point position) const noexcept
  {
    auto &from = Root();
    auto &to = GetPlacedFloats().BlockFormattingContextRoot();
    if (&from == &to)
    {
      return position;
    }

    auto mappedPosition = position;
    for (auto *containingBlock = &from; containingBlock != &to;
         containingBlock = &FormattingContext::ContainingBlock(*containingBlock))
    {
      mappedPosition.MoveBy(
        BoxGeometry::BorderBoxTopLeft(ContainingBlockGeometries().GeometryForBox(*containingBlock)));
    }
    return mappedPosition;
  }

  bool FloatingContext::IsStartPositioned(const Box &floatBox) const noexcept
  {
    krys_debug_assert(floatBox.IsFloatingPositioned());
    // Note that this returns true relative to the root of this FloatingContext and not to the PlacedFloats's
    // block formatting context root. PlacedFloats's root may be an ancestor block container with mismatching
    // inline direction.
    auto floatingBoxIsInLeftToRightDirection = Root().GetWritingMode().IsBidiLTR();
    auto floatingValue = floatBox.Style().Floating();
    return floatingValue == Float::InlineStart
           || (floatingBoxIsInLeftToRightDirection && floatingValue == Float::Left)
           || (!floatingBoxIsInLeftToRightDirection && floatingValue == Float::Right);
  }

  bool FloatingContext::IsFloatingCandidateStartPositionedInBlockFormattingContext(
    const Box &floatBox) const noexcept
  {
    krys_debug_assert(floatBox.IsFloatingPositioned());
    // A floating candidate is start positioned when:
    // - "float: inline-start or left" in left-to-right floating context with matching floating state.
    // - "float: inline-end or right" in mismatching floating state where floating state is right-to-left.
    // (FloatingContext's direction may not be the same as the PlacedFloats's direction when dealing with
    // inherited PlacedFloats across nested IFCs).
    auto floatingContextIsLeftToRight = Root().GetWritingMode().IsBidiLTR();
    auto blockFormattingContextRootIsLeftToRight =
      GetPlacedFloats().BlockFormattingContextRoot().Style().GetWritingMode().IsBidiLTR();
    if (floatingContextIsLeftToRight == blockFormattingContextRootIsLeftToRight)
    {
      return IsStartPositioned(floatBox);
    }

    auto floatingValue = floatBox.Style().Floating();
    if (floatingValue == Float::InlineStart)
    {
      floatingValue = floatingContextIsLeftToRight ? Float::Left : Float::Right;
    }
    else if (floatingValue == Float::InlineEnd)
    {
      floatingValue = floatingContextIsLeftToRight ? Float::Right : Float::Left;
    }
    return (blockFormattingContextRootIsLeftToRight && floatingValue == Float::Left)
           || (!blockFormattingContextRootIsLeftToRight && floatingValue == Float::Right);
  }

  boo::Clear FloatingContext::ClearInBlockFormattingContext(const Box &clearBox) const noexcept
  {
    // See isFloatingCandidateStartPositionedInBlockFormattingContext for details.
    krys_debug_assert(clearBox.HasFloatClear());

    auto clearBoxIsInLeftToRightDirection = Root().GetWritingMode().IsBidiLTR();
    auto clearValue = clearBox.Style().GetClear();
    if (clearValue == boo::Clear::Both)
    {
      return clearValue;
    }

    if (clearValue == boo::Clear::InlineStart)
    {
      clearValue = clearBoxIsInLeftToRightDirection ? boo::Clear::Left : boo::Clear::Right;
    }
    else if (clearValue == boo::Clear::InlineEnd)
    {
      clearValue = clearBoxIsInLeftToRightDirection ? boo::Clear::Right : boo::Clear::Left;
    }

    auto blockFormattingContextRootIsLeftToRight =
      _placedFloats.BlockFormattingContextRoot().Style().GetWritingMode().IsBidiLTR();
    return (blockFormattingContextRootIsLeftToRight && clearValue == boo::Clear::Left)
               || (!blockFormattingContextRootIsLeftToRight && clearValue == boo::Clear::Right)
             ? boo::Clear::Left
             : boo::Clear::Right;
  }

  namespace
  {
    const PlacedFloats::Item *FloatPair::InlineStart() const noexcept
    {
      if (!_floatPair.InlineStart)
      {
        return nullptr;
      }
      krys_debug_assert(_floats[*_floatPair.InlineStart].IsStartPositioned());
      return &_floats[*_floatPair.InlineStart];
    }

    const PlacedFloats::Item *FloatPair::InlineEnd() const noexcept
    {
      if (!_floatPair.InlineEnd)
      {
        return nullptr;
      }
      krys_debug_assert(!_floats[*_floatPair.InlineEnd].IsStartPositioned());
      return &_floats[*_floatPair.InlineEnd];
    }

    bool FloatPair::Intersects(const FloatAvoider &floatAvoider) const noexcept
    {
      auto intersects = [&](const PlacedFloats::Item *floating)
      {
        if (!floating)
        {
          return false;
        }

        auto floatingRect = floating->AbsoluteRectWithMargin();
        if (floatAvoider.InlineStart() >= floatingRect.Right()
            || floatAvoider.InlineEnd() <= floatingRect.Left())
        {
          return false;
        }
        return floatAvoider.BlockStart() >= floatingRect.Top()
               && floatAvoider.BlockStart() < floatingRect.Bottom();
      };

      krys_debug_assert(!_floatPair.IsEmpty());
      return intersects(InlineStart()) || intersects(InlineEnd());
    }

    bool FloatPair::Intersects(BoxGeometry::HorizontalEdges containingBlockContentBoxEdges) const noexcept
    {
      krys_debug_assert(!_floatPair.IsEmpty());
      auto inlineAxisConstraints = GetInlineAxisConstraints();
      if (inlineAxisConstraints.Start && *inlineAxisConstraints.Start > containingBlockContentBoxEdges.Start)
      {
        return true;
      }
      if (inlineAxisConstraints.End && *inlineAxisConstraints.End < containingBlockContentBoxEdges.End)
      {
        return true;
      }
      return false;
    }

    bool FloatPair::ContainsFloatFromFormattingContext() const noexcept
    {
      krys_debug_assert(!_floatPair.IsEmpty());
      auto isInsideCurrentFormattingContext = [&](const PlacedFloats::Item *floatBox)
      {
        // FIXME: This should be a tree traversal on the ancestor chain to see if this belongs to the
        // FloatingContext's formatting context (e.g. float may come from parent and sibling formatting
        // contexts)
        return floatBox && floatBox->LayoutBox();
      };
      return isInsideCurrentFormattingContext(InlineStart()) || isInsideCurrentFormattingContext(InlineEnd());
    }

    FloatPair::InlineAxisConstraints FloatPair::GetInlineAxisConstraints() const noexcept
    {
      auto startEdge = Maybe<PositionInContextRoot> {};
      auto endEdge = Maybe<PositionInContextRoot> {};

      if (auto *inlineStart = InlineStart())
      {
        startEdge = PositionInContextRoot {inlineStart->AbsoluteRectWithMargin().Right()};
      }

      if (auto *inlineEnd = InlineEnd())
      {
        endEdge = PositionInContextRoot {inlineEnd->AbsoluteRectWithMargin().Left()};
      }

      return {startEdge, endEdge};
    }

    PositionInContextRoot FloatPair::LowestBlockAxisPosition() const noexcept
    {
      auto *inlineStart = InlineStart();
      auto *inlineEnd = InlineEnd();
      krys_debug_assert(inlineStart || inlineEnd);

      auto lowestBlockAxisPositionAtInlineStart =
        inlineStart ? Maybe<PositionInContextRoot>(
                        PositionInContextRoot {inlineStart->AbsoluteRectWithMargin().Bottom()})
                    : null;
      auto lowestBlockAxisPositionAtInlineEnd =
        inlineEnd
          ? Maybe<PositionInContextRoot>(PositionInContextRoot {inlineEnd->AbsoluteRectWithMargin().Bottom()})
          : null;

      if (lowestBlockAxisPositionAtInlineStart && lowestBlockAxisPositionAtInlineEnd)
      {
        return std::max(*lowestBlockAxisPositionAtInlineStart, *lowestBlockAxisPositionAtInlineEnd);
      }

      if (lowestBlockAxisPositionAtInlineStart)
      {
        return *lowestBlockAxisPositionAtInlineStart;
      }

      return *lowestBlockAxisPositionAtInlineEnd;
    }

    Iterator::Iterator(const PlacedFloats::List &floats, Maybe<PositionInContextRoot> blockStart) noexcept
        : _floats(floats), _current(floats)
    {
      if (blockStart)
      {
        Set(*blockStart);
      }
    }

    KRYS_NODISCARD Maybe<size_t> PreviousFloatingIndex(Float floatingType, const PlacedFloats::List &floats,
                                                       size_t currentIndex) noexcept
    {
      krys_debug_assert(floatingType == Float::InlineStart || floatingType == Float::InlineEnd);
      krys_assert(currentIndex <= floats.size());

      while (currentIndex)
      {
        auto &floating = floats[--currentIndex];
        if ((floatingType == Float::InlineStart && floating.IsStartPositioned())
            || (floatingType == Float::InlineEnd && !floating.IsStartPositioned()))
        {
          return currentIndex;
        }
      }

      return null;
    }

    Iterator &Iterator::operator++() noexcept
    {
      if (_current.IsEmpty())
      {
        krys_debug_assert(false);
        return *this;
      }

      auto findPreviousFloatingWithLowerOnBlockAxis = [&](Float floatingType,
                                                          size_t currentIndex) -> Maybe<size_t>
      {
        krys_debug_assert(floatingType == Float::InlineStart || floatingType == Float::InlineEnd);
        krys_assert(currentIndex < _floats.size());

        // Last floating? There's certainly no previous floating at this point.
        if (!currentIndex)
        {
          return null;
        }

        auto currentBlockAxisPosition = _floats[currentIndex].AbsoluteRectWithMargin().Bottom();

        Maybe<size_t> index = currentIndex;
        while (true)
        {
          index = PreviousFloatingIndex(floatingType, _floats, *index);
          if (!index)
          {
            return null;
          }

          if (_floats[*index].AbsoluteRectWithMargin().Bottom() > currentBlockAxisPosition)
          {
            return index;
          }
        }
      };

      // 1. Take the current floating from left and right and check which one's bottom edge is positioned
      // higher (they could be on the same vertical position too). The current floats from left and right are
      // considered the inner-most pair for the current vertical position.
      // 2. Move away from inner-most pair by picking one of the previous floats in the list(#1)
      // Ensure that the new floating bottom edge is positioned lower than the current one -which essentially
      // means skipping in-between floats that are positioned higher).
      // 3. Reset the vertical position and align it with the new left-right pair. These floats are now the
      // inner-most boxes for the current vertical position. As the result we have more horizontal space on
      // the current vertical position.
      auto lowestBlockAxisPositionAtInlineStart =
        _current.InlineStart() ? Maybe<PositionInContextRoot>(_current.InlineStart()->AbsoluteBottom())
                               : null;
      auto lowestBlockAxisPositionAtInlineEnd =
        _current.InlineEnd() ? Maybe<PositionInContextRoot>(_current.InlineEnd()->AbsoluteBottom()) : null;

      auto updateInlineStartSide =
        (lowestBlockAxisPositionAtInlineStart == lowestBlockAxisPositionAtInlineEnd)
        || (!lowestBlockAxisPositionAtInlineEnd
            || (lowestBlockAxisPositionAtInlineStart
                && *lowestBlockAxisPositionAtInlineStart < *lowestBlockAxisPositionAtInlineEnd));
      auto updateInlineEndSide =
        (lowestBlockAxisPositionAtInlineStart == lowestBlockAxisPositionAtInlineEnd)
        || (!lowestBlockAxisPositionAtInlineStart
            || (lowestBlockAxisPositionAtInlineEnd
                && *lowestBlockAxisPositionAtInlineStart > *lowestBlockAxisPositionAtInlineEnd));

      if (updateInlineStartSide)
      {
        krys_debug_assert(_current._floatPair.InlineStart);
        _current._highestBlockAxisPosition = *lowestBlockAxisPositionAtInlineStart;
        _current._floatPair.InlineStart =
          findPreviousFloatingWithLowerOnBlockAxis(Float::InlineStart, *_current._floatPair.InlineStart);
      }

      if (updateInlineEndSide)
      {
        krys_debug_assert(_current._floatPair.InlineEnd);
        _current._highestBlockAxisPosition = *lowestBlockAxisPositionAtInlineEnd;
        _current._floatPair.InlineEnd =
          findPreviousFloatingWithLowerOnBlockAxis(Float::InlineEnd, *_current._floatPair.InlineEnd);
      }

      return *this;
    }

    void Iterator::Set(PositionInContextRoot blockAxisPosition) noexcept
    {
      // Move the iterator to the initial vertical position by starting at the inner-most floating pair (last
      // floats on left/right).
      // 1. Check if the inner-most pair covers the vertical position.
      // 2. Move outwards from the inner-most pair until the vertical position intersects.
      _current._highestBlockAxisPosition = blockAxisPosition;

      // No floats at all?
      if (_floats.empty())
      {
        krys_debug_assert(false);

        _current._floatPair = {};
        return;
      }

      auto findFloatingBelow = [&](Float floatingType) -> Maybe<size_t>
      {
        krys_debug_assert(floatingType == Float::InlineStart || floatingType == Float::InlineEnd);
        krys_debug_assert(!_floats.empty());

        auto index = floatingType == Float::InlineStart ? _current._floatPair.InlineStart
                                                        : _current._floatPair.InlineEnd;
        // Start from the end if we don't have current yet.
        index = index.value_or(_floats.size());
        while (true)
        {
          index = PreviousFloatingIndex(floatingType, _floats, *index);
          if (!index)
          {
            return null;
          }

          // Is this floating intrusive on this position?
          auto rect = _floats[*index].AbsoluteRectWithMargin();
          if (rect.Top() <= blockAxisPosition && rect.Bottom() > blockAxisPosition)
          {
            return index;
          }
        }
      };

      _current._floatPair.InlineStart = findFloatingBelow(Float::InlineStart);
      _current._floatPair.InlineEnd = findFloatingBelow(Float::InlineEnd);

      krys_debug_assert(!_current._floatPair.InlineStart
                        || (*_current._floatPair.InlineStart < _floats.size()
                            && _floats[*_current._floatPair.InlineStart].IsStartPositioned()));
      krys_debug_assert(!_current._floatPair.InlineEnd
                        || (*_current._floatPair.InlineEnd < _floats.size()
                            && !_floats[*_current._floatPair.InlineEnd].IsStartPositioned()));
    }
  }
}
