#include "Krystal.Booey/Platform/Scrolling/ScrollAlignment.hpp"
#include <algorithm>

namespace krys::boo
{
  using Behavior = ScrollAlignment::Behavior;

  const ScrollAlignment ScrollAlignment::NoScroll = {Behavior::NoScroll, Behavior::NoScroll,
                                                     Behavior::NoScroll};
  const ScrollAlignment ScrollAlignment::AlignCenterIfNotVisible = {Behavior::NoScroll, Behavior::AlignCenter,
                                                                    Behavior::NoScroll};
  const ScrollAlignment ScrollAlignment::AlignToEdgeIfNotVisible = {
    Behavior::NoScroll, Behavior::AlignToClosestEdge, Behavior::NoScroll};
  const ScrollAlignment ScrollAlignment::AlignCenterIfNeeded = {Behavior::NoScroll, Behavior::AlignCenter,
                                                                Behavior::AlignToClosestEdge};
  const ScrollAlignment ScrollAlignment::AlignToEdgeIfNeeded = {
    Behavior::NoScroll, Behavior::AlignToClosestEdge, Behavior::AlignToClosestEdge};
  const ScrollAlignment ScrollAlignment::AlignCenterAlways = {Behavior::AlignCenter, Behavior::AlignCenter,
                                                              Behavior::AlignCenter};
  const ScrollAlignment ScrollAlignment::AlignTopAlways = {Behavior::AlignTop, Behavior::AlignTop,
                                                           Behavior::AlignTop};
  const ScrollAlignment ScrollAlignment::AlignRightAlways = {Behavior::AlignRight, Behavior::AlignRight,
                                                             Behavior::AlignRight};
  const ScrollAlignment ScrollAlignment::AlignLeftAlways = {Behavior::AlignLeft, Behavior::AlignLeft,
                                                            Behavior::AlignLeft};
  const ScrollAlignment ScrollAlignment::AlignBottomAlways = {Behavior::AlignBottom, Behavior::AlignBottom,
                                                              Behavior::AlignBottom};

  namespace
  {
    /// @brief One axis of a rect: where it starts and how long it is.
    struct ScrollAxisSpan
    {
      LayoutUnit Start;
      LayoutUnit Length;

      KRYS_NODISCARD LayoutUnit End() const noexcept
      {
        return Start + Length;
      }
    };

    /// @brief Where the visible span should start along one axis so @p expose is placed as @p alignment asks.
    /// @param endBehavior The behaviour that aligns the end edge on this axis (right or bottom).
    /// @param visibleThreshold How much of @p expose showing counts as all of it, if anything.
    /// @param roundCenterUp Whether centring rounds the half-difference up to a whole pixel.
    KRYS_NODISCARD LayoutUnit ScrollPositionForAxis(ScrollAxisSpan visible, ScrollAxisSpan expose,
                                                    ScrollAxisSpan visibilityCheck,
                                                    const ScrollAlignment &alignment, Behavior endBehavior,
                                                    Maybe<LayoutUnit> visibleThreshold,
                                                    bool roundCenterUp) noexcept
    {
      auto behavior = alignment.RectHidden;
      if (visibilityCheck.End() >= visible.Start && visibilityCheck.Start <= visible.End())
      {
        auto shown = std::max(LayoutUnit(),
                              std::min(visible.End(), expose.End()) - std::max(visible.Start, expose.Start));
        if (shown == expose.Length || shown == visible.Length
            || (visibleThreshold && shown >= *visibleThreshold))
        {
          behavior = alignment.RectVisible;
        }
        else if (shown > 0)
        {
          behavior = alignment.RectPartial;
        }
      }

      // https://drafts.csswg.org/cssom-view-1/#determine-the-scroll-into-view-position ("nearest")
      // The end edge is aligned when it's outside and the rect is smaller than the visible span, or when the
      // start edge is outside and the rect is bigger; otherwise the start edge is.
      if (behavior == Behavior::AlignToClosestEdge
          && ((expose.End() > visible.End() && expose.Length < visible.Length)
              || (expose.Start < visible.Start && expose.Length > visible.Length)))
      {
        behavior = endBehavior;
      }

      if (behavior == Behavior::NoScroll)
      {
        return visible.Start;
      }
      if (behavior == endBehavior)
      {
        return expose.End() - visible.Length;
      }
      if (behavior == Behavior::AlignCenter)
      {
        auto half = (expose.Length - visible.Length) / 2;
        return expose.Start + (roundCenterUp ? LayoutUnit(half.Ceil()) : half);
      }
      // Start edges, and the other axis's end edges, which mean nothing on this one.
      return expose.Start;
    }
  }

  LayoutRect GetRectToExposeForScrollIntoView(const LayoutRect &visibleBounds, const LayoutRect &exposeRect,
                                              const ScrollAlignment &alignX, const ScrollAlignment &alignY,
                                              const Maybe<LayoutRect> visibilityCheckRect) noexcept
  {
    // At least this much of a rect showing horizontally counts as all of it, while the threshold is enabled.
    constexpr int32 MinIntersectForReveal = 32;

    const auto &checkRect = visibilityCheckRect ? *visibilityCheckRect : exposeRect;
    auto x = ScrollPositionForAxis(
      {visibleBounds.X(), visibleBounds.Width()}, {exposeRect.X(), exposeRect.Width()},
      {checkRect.X(), checkRect.Width()}, alignX, Behavior::AlignRight,
      alignX.EnableLegacyHorizontalVisibilityThreshold ? Maybe<LayoutUnit>(LayoutUnit(MinIntersectForReveal))
                                                       : null,
      false);
    auto y = ScrollPositionForAxis({visibleBounds.Y(), visibleBounds.Height()},
                                   {exposeRect.Y(), exposeRect.Height()}, {checkRect.Y(), checkRect.Height()},
                                   alignY, Behavior::AlignBottom, null, true);
    return LayoutRect(LayoutPoint(x, y), visibleBounds.Size());
  }
}
