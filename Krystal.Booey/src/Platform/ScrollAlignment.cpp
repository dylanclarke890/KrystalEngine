#include "Krystal.Booey/Platform/ScrollAlignment.hpp"
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

  LayoutRect GetRectToExposeForScrollIntoView(const LayoutRect &visibleBounds, const LayoutRect &exposeRect,
                                              const ScrollAlignment &alignX, const ScrollAlignment &alignY,
                                              const Maybe<LayoutRect> visibilityCheckRect) noexcept
  {
    constexpr int32 MinIntersectForReveal = 32;

    auto scrollX = alignX.GetHiddenBehavior();

    bool intersectsInX = false;
    if (visibilityCheckRect)
    {
      intersectsInX =
        visibilityCheckRect->MaxX() >= visibleBounds.X() && visibilityCheckRect->X() <= visibleBounds.MaxX();
    }
    else
    {
      intersectsInX = exposeRect.MaxX() >= visibleBounds.X() && exposeRect.X() <= visibleBounds.MaxX();
    }

    // Determine the appropriate X behavior.
    if (intersectsInX)
    {
      auto intersectWidth = std::max(LayoutUnit(), std::min(visibleBounds.MaxX(), exposeRect.MaxX())
                                                     - std::max(visibleBounds.X(), exposeRect.X()));

      if (intersectWidth == exposeRect.Width()
          || (alignX.LegacyHorizontalVisibilityThresholdEnabled() && intersectWidth >= MinIntersectForReveal))
      {
        // If the rectangle is fully visible, use the specified visible behavior.
        // If the rectangle is partially visible, but over a certain threshold,
        // then treat it as fully visible to avoid unnecessary horizontal scrolling
        scrollX = alignX.GetVisibleBehavior();
      }
      else if (intersectWidth == visibleBounds.Width())
      {
        // The rect is bigger than the visible area.
        scrollX = alignX.GetVisibleBehavior();
      }
      else if (intersectWidth > 0)
      {
        // If the rectangle is partially visible, but not above the minimum threshold, use the specified
        // partial behavior
        scrollX = alignX.GetPartialBehavior();
      }
      else
      {
        scrollX = alignX.GetHiddenBehavior();
      }
    }

    // If we're trying to align to the closest edge, and the exposeRect is further right
    // than the visibleBounds, and not bigger than the visible area, then align with the right.
    if (scrollX == Behavior::AlignToClosestEdge && exposeRect.MaxX() > visibleBounds.MaxX()
        && exposeRect.Width() < visibleBounds.Width())
    {
      scrollX = Behavior::AlignRight;
    }

    // Given the X behavior, compute the X coordinate.
    LayoutUnit x;
    if (scrollX == Behavior::NoScroll)
    {
      x = visibleBounds.X();
    }
    else if (scrollX == Behavior::AlignRight)
    {
      x = exposeRect.MaxX() - visibleBounds.Width();
    }
    else if (scrollX == Behavior::AlignCenter)
    {
      x = exposeRect.X() + (exposeRect.Width() - visibleBounds.Width()) / 2;
    }
    else
    {
      x = exposeRect.X();
    }

    auto scrollY = alignY.GetHiddenBehavior();

    bool intersectsInY = false;
    if (visibilityCheckRect)
    {
      intersectsInY =
        visibilityCheckRect->MaxY() >= visibleBounds.Y() && visibilityCheckRect->Y() <= visibleBounds.MaxY();
    }
    else
    {
      intersectsInY = exposeRect.MaxY() >= visibleBounds.Y() && exposeRect.Y() <= visibleBounds.MaxY();
    }

    // Determine the appropriate Y behavior.
    if (intersectsInY)
    {
      auto intersectHeight = std::max(LayoutUnit(), std::min(visibleBounds.MaxY(), exposeRect.MaxY())
                                                      - std::max(visibleBounds.Y(), exposeRect.Y()));
      if (intersectHeight == exposeRect.Height())
      {
        // If the rectangle is fully visible, use the specified visible behavior.
        scrollY = alignY.GetVisibleBehavior();
      }
      else if (intersectHeight == visibleBounds.Height())
      {
        // The rect is bigger than the visible area.
        scrollY = alignY.GetVisibleBehavior();
      }
      else if (intersectHeight > 0)
      {
        // If the rectangle is partially visible, use the specified partial behavior
        scrollY = alignY.GetPartialBehavior();
      }
      else
      {
        scrollY = alignY.GetHiddenBehavior();
      }
    }

    // If we're trying to align to the closest edge, and the exposeRect is further down
    // than the visibleBounds, and not bigger than the visible area, then align with the bottom.
    if (scrollY == Behavior::AlignToClosestEdge && exposeRect.MaxY() > visibleBounds.MaxY()
        && exposeRect.Height() < visibleBounds.Height())
    {
      scrollY = Behavior::AlignBottom;
    }

    // Given the Y behavior, compute the Y coordinate.
    LayoutUnit y;
    if (scrollY == Behavior::NoScroll)
    {
      y = visibleBounds.Y();
    }
    else if (scrollY == Behavior::AlignBottom)
    {
      y = exposeRect.MaxY() - visibleBounds.Height();
    }
    else if (scrollY == Behavior::AlignCenter)
    {
      auto halfHeight = (exposeRect.Height() - visibleBounds.Height()) / 2;
      y = exposeRect.Y() + LayoutUnit(halfHeight.Ceil());
    }
    else
    {
      y = exposeRect.Y();
    }

    return LayoutRect(LayoutPoint(x, y), visibleBounds.Size());
  }
}
