#include "Krystal.Booey/Page/Scrolling/ScrollSnapOffsetsInfo.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatQuad.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollableArea.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollAnimator.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollMargin.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollPadding.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  namespace
  {
    using SnapCandidate = std::pair<LayoutUnit, uint32>;

    KRYS_NODISCARD std::pair<LayoutUnit, LayoutUnit> RangeForAxis(const LayoutRect &rect,
                                                                  ScrollEventAxis axis) noexcept
    {
      return axis == ScrollEventAxis::Horizontal ? std::pair(rect.X(), rect.MaxX())
                                                 : std::pair(rect.Y(), rect.MaxY());
    }

    KRYS_NODISCARD LayoutUnit ComponentForAxis(const LayoutPoint &point, ScrollEventAxis axis) noexcept
    {
      return axis == ScrollEventAxis::Horizontal ? point.X() : point.Y();
    }

    KRYS_NODISCARD LayoutUnit LengthForAxis(const LayoutSize &size, ScrollEventAxis axis) noexcept
    {
      return axis == ScrollEventAxis::Horizontal ? size.Width() : size.Height();
    }

    KRYS_NODISCARD constexpr ScrollEventAxis OtherAxis(ScrollEventAxis axis) noexcept
    {
      return axis == ScrollEventAxis::Horizontal ? ScrollEventAxis::Vertical : ScrollEventAxis::Horizontal;
    }

    struct PotentialSnapPointSearchResult
    {
      Maybe<SnapCandidate> Previous;
      Maybe<SnapCandidate> Next;
      Maybe<SnapCandidate> SnapStop;
      bool LandedInsideSnapAreaThatConsumesViewport {false};
    };

    KRYS_NODISCARD PotentialSnapPointSearchResult SearchForPotentialSnapPoints(
      const ScrollSnapOffsetsInfo &info, ScrollEventAxis axis, LayoutUnit viewportLength,
      LayoutUnit destinationOffset, Maybe<LayoutUnit> originalOffset) noexcept
    {
      const auto &snapOffsets = info.OffsetsForAxis(axis);
      PotentialSnapPointSearchResult result;
      Maybe<SnapCandidate> exact;

      // A snap stop is better if it's between the original and destination offsets and closer to the
      // original: a scroll always stops at the snap stop closest to where it started.
      auto isBetterSnapStop = [&](LayoutUnit candidate)
      {
        if (!originalOffset)
        {
          return false;
        }
        auto original = *originalOffset;
        if (candidate <= std::min(destinationOffset, original)
            || candidate >= std::max(destinationOffset, original))
        {
          return false;
        }
        return !result.SnapStop
               || std::abs(static_cast<float>(candidate - original))
                    < std::abs(static_cast<float>(result.SnapStop->first - original));
      };

      for (uint32 i = 0; i < snapOffsets.size(); ++i)
      {
        const auto &snapOffset = snapOffsets[i];
        if (!result.LandedInsideSnapAreaThatConsumesViewport && snapOffset.HasSnapAreaLargerThanViewport)
        {
          for (auto areaIndex : snapOffset.SnapAreaIndices)
          {
            auto [areaMin, areaMax] = RangeForAxis(info.SnapAreas[areaIndex], axis);
            if (areaMin <= destinationOffset && areaMax >= destinationOffset + viewportLength)
            {
              result.LandedInsideSnapAreaThatConsumesViewport = true;
              break;
            }
          }
        }

        if (snapOffset.Offset == destinationOffset)
        {
          exact = SnapCandidate(snapOffset.Offset, i);
        }
        else if (snapOffset.Offset < destinationOffset)
        {
          result.Previous = SnapCandidate(snapOffset.Offset, i);
        }
        else if (!result.Next)
        {
          result.Next = SnapCandidate(snapOffset.Offset, i);
        }

        if (snapOffset.Stop == ScrollSnapStop::Always && isBetterSnapStop(snapOffset.Offset))
        {
          result.SnapStop = SnapCandidate(snapOffset.Offset, i);
        }
      }

      if (exact)
      {
        result.Previous = exact;
        result.Next = exact;
      }
      return result;
    }

    KRYS_NODISCARD bool OffsetHasVisibleSnapArea(const ScrollSnapOffsetsInfo &info,
                                                 const SnapOffset &snapOffset, const SnapOffset &otherOffset,
                                                 ScrollEventAxis axis,
                                                 const LayoutSize &viewportSize) noexcept
    {
      auto otherAxis = OtherAxis(axis);
      auto otherLength = LengthForAxis(viewportSize, otherAxis);
      for (auto index : snapOffset.SnapAreaIndices)
      {
        auto [otherMin, otherMax] = RangeForAxis(info.SnapAreas[index], otherAxis);
        if (otherOffset.Offset + otherLength > otherMin && otherOffset.Offset < otherMax)
        {
          return true;
        }
      }
      return false;
    }

    /// @return The index in @p snapOffset's snap areas of one that would be visible, if there is one.
    KRYS_NODISCARD Maybe<uint32> FindCompatibleSnapArea(const ScrollSnapOffsetsInfo &info,
                                                        const SnapOffset &snapOffset, ScrollEventAxis axis,
                                                        const LayoutSize &viewportSize,
                                                        const LayoutPoint &destination) noexcept
    {
      auto otherAxis = OtherAxis(axis);
      auto viewportLength = LengthForAxis(viewportSize, axis);
      auto otherLength = LengthForAxis(viewportSize, otherAxis);
      auto otherDestination = ComponentForAxis(destination, otherAxis);
      for (uint32 i = 0; i < snapOffset.SnapAreaIndices.size(); ++i)
      {
        const auto &area = info.SnapAreas[snapOffset.SnapAreaIndices[i]];
        auto [axisMin, axisMax] = RangeForAxis(area, axis);
        auto [otherMin, otherMax] = RangeForAxis(area, otherAxis);
        if (info.OffsetsForAxis(otherAxis).empty()
            && (otherDestination + otherLength < otherMin || otherDestination > otherMax))
        {
          continue;
        }
        if (snapOffset.Offset + viewportLength > axisMin && snapOffset.Offset < axisMax)
        {
          return i;
        }
      }
      return null;
    }

    void AdjustPreviousAndNextForOnScreenSnapAreas(const ScrollSnapOffsetsInfo &info, ScrollEventAxis axis,
                                                   const LayoutSize &viewportSize,
                                                   const LayoutPoint &destination,
                                                   PotentialSnapPointSearchResult &searchResult) noexcept
    {
      const auto &snapOffsets = info.OffsetsForAxis(axis);
      if (searchResult.Previous)
      {
        uint32 oldIndex = searchResult.Previous->second;
        searchResult.Previous = null;
        for (uint32 offset = 0; offset <= oldIndex; ++offset)
        {
          uint32 index = oldIndex - offset;
          if (FindCompatibleSnapArea(info, snapOffsets[index], axis, viewportSize, destination))
          {
            searchResult.Previous = SnapCandidate(snapOffsets[index].Offset, index);
            break;
          }
        }
      }

      if (searchResult.Next)
      {
        uint32 oldIndex = searchResult.Next->second;
        searchResult.Next = null;
        for (uint32 index = oldIndex; index < snapOffsets.size(); ++index)
        {
          if (FindCompatibleSnapArea(info, snapOffsets[index], axis, viewportSize, destination))
          {
            searchResult.Next = SnapCandidate(snapOffsets[index].Offset, index);
            break;
          }
        }
      }
    }

    using SnapResult = std::pair<LayoutUnit, Maybe<uint32>>;

    KRYS_NODISCARD SnapResult ClosestSnapOffsetOnAxis(const ScrollSnapOffsetsInfo &info, ScrollEventAxis axis,
                                                      const LayoutSize &viewportSize,
                                                      const LayoutPoint &destination, float velocity,
                                                      Maybe<LayoutUnit> originalOffset) noexcept
    {
      auto destinationOffset = ComponentForAxis(destination, axis);
      const auto &snapOffsets = info.OffsetsForAxis(axis);
      SnapResult noSnapping(destinationOffset, null);
      if (snapOffsets.empty())
      {
        return noSnapping;
      }

      auto toResult = [](const SnapCandidate &candidate)
      {
        return SnapResult(candidate.first, candidate.second);
      };

      auto viewportLength = LengthForAxis(viewportSize, axis);
      auto searchResult =
        SearchForPotentialSnapPoints(info, axis, viewportLength, destinationOffset, originalOffset);
      if (searchResult.SnapStop)
      {
        return toResult(*searchResult.SnapStop);
      }

      AdjustPreviousAndNextForOnScreenSnapAreas(info, axis, viewportSize, destination, searchResult);
      auto &previous = searchResult.Previous;
      auto &next = searchResult.Next;

      // https://drafts.csswg.org/css-scroll-snap-1/#snap-overflow
      // A position in which a snap area larger than the snapport covers it is valid, if the snap positions
      // either side are further apart than the snapport.
      if (searchResult.LandedInsideSnapAreaThatConsumesViewport
          && (!previous || !next || next->first - previous->first >= viewportLength))
      {
        return noSnapping;
      }

      auto isNearEnoughForProximity = [&](LayoutUnit candidate)
      {
        if (info.Strictness != ScrollSnapStrictness::Proximity)
        {
          return true;
        }
        // What's "in proximity" is the user agent's to choose.
        constexpr float ratioOfSnapportForProximity = 0.3f;
        return std::abs(static_cast<float>(candidate - destinationOffset))
               <= static_cast<float>(viewportLength) * ratioOfSnapportForProximity;
      };

      if (destinationOffset <= snapOffsets.front().Offset)
      {
        return isNearEnoughForProximity(snapOffsets.front().Offset)
                 ? SnapResult(snapOffsets.front().Offset, 0u)
                 : noSnapping;
      }

      if (destinationOffset >= snapOffsets.back().Offset)
      {
        auto lastIndex = static_cast<uint32>(snapOffsets.size() - 1);
        return isNearEnoughForProximity(snapOffsets.back().Offset)
                 ? SnapResult(snapOffsets.back().Offset, lastIndex)
                 : noSnapping;
      }

      if (previous && !isNearEnoughForProximity(previous->first))
      {
        previous = null;
      }
      if (next && !isNearEnoughForProximity(next->first))
      {
        next = null;
      }

      if (originalOffset)
      {
        // https://drafts.csswg.org/css-scroll-snap-1/#choosing
        // A user must be able to escape a snap position, so a directional scroll never snaps back to where it
        // started or before it.
        if (*originalOffset < destinationOffset && previous && previous->first <= *originalOffset)
        {
          previous = null;
        }
        if (*originalOffset > destinationOffset && next && next->first >= *originalOffset)
        {
          next = null;
        }
      }

      if (!previous && !next)
      {
        return noSnapping;
      }
      if (!previous)
      {
        return toResult(*next);
      }
      if (!next)
      {
        return toResult(*previous);
      }

      // A scroll without a direction takes whichever is closer; otherwise, the one in its direction.
      if (!velocity)
      {
        return toResult(destinationOffset - previous->first <= next->first - destinationOffset ? *previous
                                                                                               : *next);
      }
      return toResult(velocity < 0 ? *previous : *next);
    }

    KRYS_NODISCARD LayoutUnit SnapAlignOffset(LayoutUnit minLocation, LayoutUnit maxLocation,
                                              ScrollSnapAxisAlignType alignment, bool axisIsFlipped) noexcept
    {
      switch (alignment)
      {
        case ScrollSnapAxisAlignType::Start:  return axisIsFlipped ? maxLocation : minLocation;
        case ScrollSnapAxisAlignType::Center: return (minLocation + maxLocation) / 2;
        case ScrollSnapAxisAlignType::End:    return axisIsFlipped ? minLocation : maxLocation;
        case ScrollSnapAxisAlignType::None:   break;
      }
      krys_debug_assert(false);
      return 0;
    }

    void AddOrUpdateSnapOffset(List<SnapOffset> &offsets, LayoutUnit newOffset, ScrollSnapStop stop,
                               bool hasSnapAreaLargerThanViewport, uint32 snapAreaIndex) noexcept
    {
      auto found = std::find_if(offsets.begin(), offsets.end(),
                                [&](const SnapOffset &offset) { return offset.Offset == newOffset; });
      if (found == offsets.end())
      {
        offsets.push_back({newOffset, stop, hasSnapAreaLargerThanViewport, {snapAreaIndex}});
        return;
      }
      if (stop == ScrollSnapStop::Always)
      {
        found->Stop = ScrollSnapStop::Always;
      }
      found->HasSnapAreaLargerThanViewport |= hasSnapAreaLargerThanViewport;
      found->SnapAreaIndices.push_back(snapAreaIndex);
    }
  }

  std::pair<LayoutUnit, Maybe<uint32>>
    ScrollSnapOffsetsInfo::ClosestSnapOffset(ScrollEventAxis axis, const LayoutSize &viewportSize,
                                             LayoutPoint destination, float velocity,
                                             Maybe<LayoutUnit> originalOffset) const noexcept
  {
    auto horizontal =
      ClosestSnapOffsetOnAxis(*this, ScrollEventAxis::Horizontal, viewportSize, destination, velocity,
                              axis == ScrollEventAxis::Horizontal ? originalOffset : null);
    auto vertical =
      ClosestSnapOffsetOnAxis(*this, ScrollEventAxis::Vertical, viewportSize, destination, velocity,
                              axis == ScrollEventAxis::Vertical ? originalOffset : null);

    // If neither axis's snap area would be visible at the pair of offsets, both go to the closer of the two
    // areas.
    if (horizontal.second && vertical.second
        && !OffsetHasVisibleSnapArea(*this, HorizontalSnapOffsets[*horizontal.second],
                                     VerticalSnapOffsets[*vertical.second], ScrollEventAxis::Horizontal,
                                     viewportSize)
        && !OffsetHasVisibleSnapArea(*this, VerticalSnapOffsets[*vertical.second],
                                     HorizontalSnapOffsets[*horizontal.second], ScrollEventAxis::Vertical,
                                     viewportSize))
    {
      const auto &horizontalOffset = HorizontalSnapOffsets[*horizontal.second];
      const auto &verticalOffset = VerticalSnapOffsets[*vertical.second];
      auto horizontalArea = FindCompatibleSnapArea(*this, horizontalOffset, ScrollEventAxis::Horizontal,
                                                   viewportSize, destination);
      auto verticalArea =
        FindCompatibleSnapArea(*this, verticalOffset, ScrollEventAxis::Vertical, viewportSize, destination);
      if (horizontalArea && verticalArea)
      {
        const auto &first = SnapAreas[horizontalOffset.SnapAreaIndices[*horizontalArea]];
        const auto &second = SnapAreas[verticalOffset.SnapAreaIndices[*verticalArea]];
        auto squaredDistance = [&](const LayoutRect &area)
        {
          float x = static_cast<float>(area.X() - destination.X());
          float y = static_cast<float>(area.Y() - destination.Y());
          return x * x + y * y;
        };
        const auto &closer = squaredDistance(first) > squaredDistance(second) ? second : first;
        horizontal = SnapResult(closer.X(), null);
        vertical = SnapResult(closer.Y(), null);
      }
    }
    return axis == ScrollEventAxis::Horizontal ? horizontal : vertical;
  }

  void UpdateSnapOffsetsForScrollableArea(ScrollableArea &scrollableArea,
                                          const RenderBox &scrollingElementBox,
                                          const LayoutRect &viewportRect) noexcept
  {
    const auto &style = scrollingElementBox.Style();
    auto container = style.GetScrollSnapType().TryContainer();
    const auto &boxes = scrollingElementBox.View().BoxesWithScrollSnapPositions();
    if (!container || boxes.empty())
    {
      scrollableArea.ClearSnapOffsets();
      return;
    }

    auto writingMode = style.GetWritingMode();
    bool scrollerXAxisFlipped = !writingMode.IsAnyLeftToRight();
    bool scrollerYAxisFlipped = !writingMode.IsAnyTopToBottom();
    bool scrollerIsVertical = writingMode.IsVertical();
    bool hasHorizontalSnapOffsets =
      container->Axis == ScrollSnapAxis::Both || container->Axis == ScrollSnapAxis::XAxis;
    bool hasVerticalSnapOffsets =
      container->Axis == ScrollSnapAxis::Both || container->Axis == ScrollSnapAxis::YAxis;
    if (container->Axis == ScrollSnapAxis::Block)
    {
      hasHorizontalSnapOffsets = scrollerIsVertical;
      hasVerticalSnapOffsets = !scrollerIsVertical;
    }
    if (container->Axis == ScrollSnapAxis::Inline)
    {
      hasHorizontalSnapOffsets = !scrollerIsVertical;
      hasVerticalSnapOffsets = scrollerIsVertical;
    }

    auto maxScrollOffset = scrollableArea.MaximumScrollOffset();
    maxScrollOffset.ClampNegativeToZero();
    auto scrollPosition = LayoutPoint(scrollableArea.GetScrollPosition());

    // The scroll container's snapport, where the top left of its border box is the origin.
    auto scrollSnapPort = viewportRect;
    scrollSnapPort.Contract(
      style::ExtentForRect(style.GetScrollPaddingBox(), viewportRect, style::ZoomNeeded {}));

    ScrollSnapOffsetsInfo info;
    info.Strictness = container->Strictness;

    for (const auto *child : boxes)
    {
      if (child->EnclosingScrollableContainer() != &scrollingElementBox || !child->GetElement())
      {
        continue;
      }

      // The snap area is the bounding box of the element's border box, transformed.
      auto scrollSnapArea = LayoutRect(
        child
          ->LocalToContainerQuad(FloatQuad(FloatRect(child->BorderBoundingBox())), &scrollingElementBox,
                                 {MapCoordinatesMode::UseTransforms, MapCoordinatesMode::IgnoreStickyOffsets})
          .BoundingBox());

      // That moved it by the scroll position, except for a view's.
      if (!scrollableArea.IsScrollView())
      {
        scrollSnapArea.MoveBy(scrollPosition);
      }

      scrollSnapArea.Expand(style::ExtentForRect(child->Style().GetScrollMarginBox(), scrollSnapArea));
      auto alignment = child->Style().GetScrollSnapAlign();
      auto stop = child->Style().GetScrollSnapStop();

      // https://drafts.csswg.org/css-scroll-snap-1/#scroll-snap-align
      // Start and end are for the snap container's writing mode, unless the snap area is larger than the
      // snapport: then they're for the box's own.
      bool areaXAxisFlipped = scrollerXAxisFlipped;
      bool areaYAxisFlipped = scrollerYAxisFlipped;
      auto childWritingMode = child->GetWritingMode();
      if ((childWritingMode.IsVertical() && scrollSnapArea.Height() > scrollSnapPort.Height())
          || (!childWritingMode.IsVertical() && scrollSnapArea.Width() > scrollSnapPort.Width()))
      {
        areaXAxisFlipped = !childWritingMode.IsAnyLeftToRight();
        areaYAxisFlipped = !childWritingMode.IsAnyTopToBottom();
      }

      auto xAlign = scrollerIsVertical ? alignment.BlockAlign : alignment.InlineAlign;
      auto yAlign = scrollerIsVertical ? alignment.InlineAlign : alignment.BlockAlign;
      bool snapsHorizontally = hasHorizontalSnapOffsets && xAlign != ScrollSnapAxisAlignType::None;
      bool snapsVertically = hasVerticalSnapOffsets && yAlign != ScrollSnapAxisAlignType::None;
      if (!snapsHorizontally && !snapsVertically)
      {
        continue;
      }

      // The snap area is defined by its scroll position, so make it relative to the scroll offsets.
      auto areaOriginRelativeToSnapPort = scrollSnapArea.Location() - scrollSnapPort.Location();
      LayoutRect areaAsOffsets(
        LayoutPoint(scrollableArea.ScrollOffsetFromPosition(RoundedIntPoint(
          LayoutPoint(areaOriginRelativeToSnapPort.Width(), areaOriginRelativeToSnapPort.Height())))),
        scrollSnapArea.Size());
      info.SnapAreas.push_back(areaAsOffsets);
      info.SnapAreaTargets.push_back(CreateWeakPtr<dom::Element>(child->GetElement()));
      auto areaIndex = static_cast<uint32>(info.SnapAreas.size() - 1);

      if (snapsHorizontally)
      {
        auto position =
          SnapAlignOffset(scrollSnapArea.X(), scrollSnapArea.MaxX(), xAlign, areaXAxisFlipped)
          - SnapAlignOffset(scrollSnapPort.X(), scrollSnapPort.MaxX(), xAlign, areaXAxisFlipped);
        auto offset = std::clamp(scrollableArea.ScrollOffsetFromPosition(IntPoint(position.Round(), 0)).X(),
                                 0, maxScrollOffset.X());
        AddOrUpdateSnapOffset(info.HorizontalSnapOffsets, LayoutUnit(offset), stop,
                              areaAsOffsets.Width() > scrollSnapPort.Width(), areaIndex);
      }
      if (snapsVertically)
      {
        auto position =
          SnapAlignOffset(scrollSnapArea.Y(), scrollSnapArea.MaxY(), yAlign, areaYAxisFlipped)
          - SnapAlignOffset(scrollSnapPort.Y(), scrollSnapPort.MaxY(), yAlign, areaYAxisFlipped);
        auto offset = std::clamp(scrollableArea.ScrollOffsetFromPosition(IntPoint(0, position.Round())).Y(),
                                 0, maxScrollOffset.Y());
        AddOrUpdateSnapOffset(info.VerticalSnapOffsets, LayoutUnit(offset), stop,
                              areaAsOffsets.Height() > scrollSnapPort.Height(), areaIndex);
      }
    }

    auto byOffset = [](const SnapOffset &a, const SnapOffset &b)
    {
      return a.Offset < b.Offset;
    };
    std::sort(info.HorizontalSnapOffsets.begin(), info.HorizontalSnapOffsets.end(), byOffset);
    std::sort(info.VerticalSnapOffsets.begin(), info.VerticalSnapOffsets.end(), byOffset);

    if (info.HorizontalSnapOffsets.empty() && info.VerticalSnapOffsets.empty())
    {
      scrollableArea.ClearSnapOffsets();
      return;
    }
    scrollableArea.GetScrollAnimator().SetSnapOffsetsInfo(krys::move(info));
  }
}
