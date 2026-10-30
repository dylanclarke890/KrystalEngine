#include "Krystal.Booey/Page/SpatialNavigation.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/HTMLSelectElement.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  namespace
  {
    bool AreRectsFullyAligned(dom::FocusDirection, const LayoutRect &, const LayoutRect &) noexcept;
    bool AreRectsPartiallyAligned(dom::FocusDirection, const LayoutRect &, const LayoutRect &) noexcept;
    bool AreRectsMoreThanFullScreenApart(dom::FocusDirection, const LayoutRect &curRect,
                                         const LayoutRect &targetRect, const LayoutSize &viewSize) noexcept;
    bool IsRectInDirection(dom::FocusDirection, const LayoutRect &, const LayoutRect &) noexcept;
    void DeflateIfOverlapped(LayoutRect &, LayoutRect &) noexcept;
    void EntryAndExitPointsForDirection(dom::FocusDirection, const LayoutRect &startingRect,
                                        const LayoutRect &potentialRect, LayoutPoint &exitPoint,
                                        LayoutPoint &entryPoint) noexcept;
    bool IsScrollableNode(const dom::ContainerNode &) noexcept;

    RectsAlignment AlignmentForRects(dom::FocusDirection direction, const LayoutRect &curRect,
                                     const LayoutRect &targetRect, const LayoutSize &viewSize) noexcept
    {
      // If we found a node in full alignment, but it is too far away, ignore it.
      if (AreRectsMoreThanFullScreenApart(direction, curRect, targetRect, viewSize))
      {
        return RectsAlignment::None;
      }

      if (AreRectsFullyAligned(direction, curRect, targetRect))
      {
        return RectsAlignment::Full;
      }

      if (AreRectsPartiallyAligned(direction, curRect, targetRect))
      {
        return RectsAlignment::Partial;
      }

      return RectsAlignment::None;
    }

    KRYS_NODISCARD inline bool IsHorizontalMove(dom::FocusDirection direction) noexcept
    {
      return direction == dom::FocusDirection::Left || direction == dom::FocusDirection::Right;
    }

    KRYS_NODISCARD inline LayoutUnit Start(dom::FocusDirection direction, const LayoutRect &rect) noexcept
    {
      return IsHorizontalMove(direction) ? rect.Y() : rect.X();
    }

    KRYS_NODISCARD inline LayoutUnit Middle(dom::FocusDirection direction, const LayoutRect &rect) noexcept
    {
      LayoutPoint center(rect.Center());
      return IsHorizontalMove(direction) ? center.Y() : center.X();
    }

    KRYS_NODISCARD inline LayoutUnit End(dom::FocusDirection direction, const LayoutRect &rect) noexcept
    {
      return IsHorizontalMove(direction) ? rect.MaxY() : rect.MaxX();
    }

    // This method checks if rects |a| and |b| are fully aligned either vertically or
    // horizontally. In general, rects whose central point falls between the top or
    // bottom of each other are considered fully aligned.
    // Rects that match this criteria are preferable target nodes in move focus changing
    // operations.
    // * a = Current focused node's rect.
    // * b = Focus candidate node's rect.
    bool AreRectsFullyAligned(dom::FocusDirection direction, const LayoutRect &a,
                              const LayoutRect &b) noexcept
    {
      LayoutUnit aStart, bStart, aEnd, bEnd;

      switch (direction)
      {
        case dom::FocusDirection::Left:
          aStart = a.X();
          bEnd = b.MaxX();
          break;
        case dom::FocusDirection::Right:
          aStart = b.X();
          bEnd = a.MaxX();
          break;
        case dom::FocusDirection::Up:
          aStart = a.Y();
          bEnd = b.Y();
          break;
        case dom::FocusDirection::Down:
          aStart = b.Y();
          bEnd = a.Y();
          break;
        default: krys_debug_assert(false); return false;
      }

      if (aStart < bEnd)
      {
        return false;
      }

      aStart = Start(direction, a);
      bStart = Start(direction, b);

      LayoutUnit aMiddle = Middle(direction, a);
      LayoutUnit bMiddle = Middle(direction, b);

      aEnd = End(direction, a);
      bEnd = End(direction, b);

      // Picture of the totally aligned logic:
      //
      //     Horizontal    Vertical        Horizontal     Vertical
      //  ****************************  *****************************
      //  *  _          *   _ _ _ _  *  *         _   *      _ _    *
      //  * |_|     _   *  |_|_|_|_| *  *  _     |_|  *     |_|_|   *
      //  * |_|....|_|  *      .     *  * |_|....|_|  *       .     *
      //  * |_|    |_| (1)     .     *  * |_|    |_| (2)      .     *
      //  * |_|         *     _._    *  *        |_|  *    _ _._ _  *
      //  *             *    |_|_|   *  *             *   |_|_|_|_| *
      //  *             *            *  *             *             *
      //  ****************************  *****************************

      // Horizontal    Vertical        Horizontal     Vertical
      // ****************************  *****************************
      // *  _......_   *   _ _ _ _  *  *  _          *    _ _ _ _  *
      // * |_|    |_|  *  |_|_|_|_| *  * |_|     _   *   |_|_|_|_| *
      // * |_|    |_|  *  .         *  * |_|    |_|  *           . *
      // * |_|        (3) .         *  * |_|....|_| (4)          . *
      // *             *  ._ _      *  *             *        _ _. *
      // *             *  |_|_|     *  *             *       |_|_| *
      // *             *            *  *             *             *
      // ****************************  *****************************

      return ((bMiddle >= aStart && bMiddle <= aEnd)    // (1)
              || (aMiddle >= bStart && aMiddle <= bEnd) // (2)
              || (bStart == aStart)                     // (3)
              || (bEnd == aEnd));                       // (4)
    }

    // This method checks if |start| and |dest| have a partial intersection, either
    // horizontally or vertically.
    // * a = Current focused node's rect.
    // * b = Focus candidate node's rect.
    bool AreRectsPartiallyAligned(dom::FocusDirection direction, const LayoutRect &a,
                                  const LayoutRect &b) noexcept
    {
      LayoutUnit aStart = Start(direction, a);
      LayoutUnit bStart = Start(direction, b);
      LayoutUnit bMiddle = Middle(direction, b);
      LayoutUnit aEnd = End(direction, a);
      LayoutUnit bEnd = End(direction, b);

      // Picture of the partially aligned logic:
      //
      //    Horizontal       Vertical
      // ********************************
      // *  _            *   _ _ _      *
      // * |_|           *  |_|_|_|     *
      // * |_|.... _     *      . .     *
      // * |_|    |_|    *      . .     *
      // * |_|....|_|    *      ._._ _  *
      // *        |_|    *      |_|_|_| *
      // *        |_|    *              *
      // *               *              *
      // ********************************
      //
      // ... and variants of the above cases.
      return ((bStart >= aStart && bStart <= aEnd) || (bMiddle >= aStart && bMiddle <= aEnd)
              || (bEnd >= aStart && bEnd <= aEnd));
    }

    bool AreRectsMoreThanFullScreenApart(dom::FocusDirection direction, const LayoutRect &curRect,
                                         const LayoutRect &targetRect, const LayoutSize &viewSize) noexcept
    {
      krys_debug_assert(IsRectInDirection(direction, curRect, targetRect));

      switch (direction)
      {
        case dom::FocusDirection::Left:  return curRect.X() - targetRect.MaxX() > viewSize.Width();
        case dom::FocusDirection::Right: return targetRect.X() - curRect.MaxX() > viewSize.Width();
        case dom::FocusDirection::Up:    return curRect.Y() - targetRect.MaxY() > viewSize.Height();
        case dom::FocusDirection::Down:  return targetRect.Y() - curRect.MaxY() > viewSize.Height();
        default:                         krys_debug_assert(false); return true;
      }
    }

    // Return true if rect |a| is below |b|. False otherwise.
    KRYS_NODISCARD inline bool Below(const LayoutRect &a, const LayoutRect &b) noexcept
    {
      return a.Y() > b.MaxY();
    }

    // Return true if rect |a| is on the right of |b|. False otherwise.
    KRYS_NODISCARD inline bool RightOf(const LayoutRect &a, const LayoutRect &b) noexcept
    {
      return a.X() > b.MaxX();
    }

    bool IsRectInDirection(dom::FocusDirection direction, const LayoutRect &curRect,
                           const LayoutRect &targetRect) noexcept
    {
      switch (direction)
      {
        case dom::FocusDirection::Left:  return targetRect.MaxX() <= curRect.X();
        case dom::FocusDirection::Right: return targetRect.X() >= curRect.MaxX();
        case dom::FocusDirection::Up:    return targetRect.MaxY() <= curRect.Y();
        case dom::FocusDirection::Down:  return targetRect.Y() >= curRect.MaxY();
        default:                         krys_debug_assert(false); return false;
      }
    }

    void DeflateIfOverlapped(LayoutRect &a, LayoutRect &b) noexcept
    {
      if (!a.Intersects(b) || a.Contains(b) || b.Contains(a))
      {
        return;
      }

      LayoutUnit deflateFactor = -FudgeFactor();

      // Avoid negative width or height values.
      if ((a.Width() + 2 * deflateFactor > 0) && (a.Height() + 2 * deflateFactor > 0))
      {
        a.Inflate(deflateFactor);
      }

      if ((b.Width() + 2 * deflateFactor > 0) && (b.Height() + 2 * deflateFactor > 0))
      {
        b.Inflate(deflateFactor);
      }
    }

    bool IsScrollableNode(const dom::ContainerNode &container) noexcept
    {
      krys_debug_assert(!Is<dom::Document>(container));
      if (!container.HasChildNodes())
      {
        return false;
      }
      if (auto *renderBox = DynamicDowncast<RenderBox>(container.Renderer()))
      {
        return renderBox->CanBeScrolledAndHasScrollableArea();
      }
      return false;
    }

    // This method calculates the exitPoint from the startingRect and the entryPoint into the candidate rect.
    // The line between those 2 points is the closest distance between the 2 rects.
    void EntryAndExitPointsForDirection(dom::FocusDirection direction, const LayoutRect &startingRect,
                                        const LayoutRect &potentialRect, LayoutPoint &exitPoint,
                                        LayoutPoint &entryPoint) noexcept
    {
      switch (direction)
      {
        case dom::FocusDirection::Left:
          exitPoint.SetX(startingRect.X());
          entryPoint.SetX(potentialRect.MaxX());
          break;
        case dom::FocusDirection::Up:
          exitPoint.SetY(startingRect.Y());
          entryPoint.SetY(potentialRect.MaxY());
          break;
        case dom::FocusDirection::Right:
          exitPoint.SetX(startingRect.MaxX());
          entryPoint.SetX(potentialRect.X());
          break;
        case dom::FocusDirection::Down:
          exitPoint.SetY(startingRect.MaxY());
          entryPoint.SetY(potentialRect.Y());
          break;
        default: krys_debug_assert(false);
      }

      switch (direction)
      {
        case dom::FocusDirection::Left:
        case dom::FocusDirection::Right:
          if (Below(startingRect, potentialRect))
          {
            exitPoint.SetY(startingRect.Y());
            entryPoint.SetY(potentialRect.MaxY());
          }
          else if (Below(potentialRect, startingRect))
          {
            exitPoint.SetY(startingRect.MaxY());
            entryPoint.SetY(potentialRect.Y());
          }
          else
          {
            exitPoint.SetY(std::max(startingRect.Y(), potentialRect.Y()));
            entryPoint.SetY(exitPoint.Y());
          }
          break;
        case dom::FocusDirection::Up:
        case dom::FocusDirection::Down:
          if (RightOf(startingRect, potentialRect))
          {
            exitPoint.SetX(startingRect.X());
            entryPoint.SetX(potentialRect.MaxX());
          }
          else if (RightOf(potentialRect, startingRect))
          {
            exitPoint.SetX(startingRect.MaxX());
            entryPoint.SetX(potentialRect.X());
          }
          else
          {
            exitPoint.SetX(std::max(startingRect.X(), potentialRect.X()));
            entryPoint.SetX(exitPoint.X());
          }
          break;
        default: krys_debug_assert(false);
      }
    }
  }

  FocusCandidate::FocusCandidate(dom::Element *element, dom::FocusDirection direction) noexcept
      : Distance(MaxDistance()), Alignment(RectsAlignment::None), IsOffscreen(true),
        IsOffscreenAfterScrolling(true)
  {
    if (auto *area = DynamicDowncast<html::HTMLAreaElement>(element))
    {
      auto image = area->ImageElement();
      if (!image || !image->Renderer())
      {
        return;
      }

      VisibleNode = CreateWeakPtr(static_cast<dom::Element *>(image.get()));
      Rect = VirtualRectForAreaElementAndDirection(area, direction);
    }
    else
    {
      if (!element->Renderer())
      {
        return;
      }

      VisibleNode = CreateWeakPtr(element);
      Rect = NodeRectInAbsoluteCoordinates(*element, true /* ignore border */);
    }

    FocusableNode = CreateWeakPtr(element);
    IsOffscreen = HasOffscreenRect(*VisibleNode);
    IsOffscreenAfterScrolling = HasOffscreenRect(*VisibleNode, direction);
  }

  dom::Document *FocusCandidate::Document() const noexcept
  {
    return VisibleNode ? &VisibleNode->NodeDocument() : nullptr;
  }

  // Checks if |node| is offscreen the visible area (viewport) of its container
  // document. In case it is, one can scroll in direction or take any different
  // desired action later on.
  bool HasOffscreenRect(const dom::Node &node, dom::FocusDirection direction) noexcept
  {
    // Get the FrameView in which |node| is (which means the current viewport if |node|
    // is not in an inner document), so we can check if its content rect is visible
    // before we actually move the focus to it.
    auto *frameView = node.NodeDocument().View();
    if (!frameView)
    {
      return true;
    }

    krys_debug_assert(!frameView->LayoutContext().NeedsLayout());

    LayoutRect containerViewportRect(frameView->VisibleContentRect());
    // We want to select a node if it is currently off screen, but will be
    // exposed after we scroll. Adjust the viewport to post-scrolling position.
    // If the container has overflow:hidden, we cannot scroll, so we do not pass direction
    // and we do not adjust for scrolling.
    switch (direction)
    {
      case dom::FocusDirection::Left:
        containerViewportRect.SetX(containerViewportRect.X() - Scrollbar::PixelsPerLineStep());
        containerViewportRect.SetWidth(containerViewportRect.Width() + Scrollbar::PixelsPerLineStep());
        break;
      case dom::FocusDirection::Right:
        containerViewportRect.SetWidth(containerViewportRect.Width() + Scrollbar::PixelsPerLineStep());
        break;
      case dom::FocusDirection::Up:
        containerViewportRect.SetY(containerViewportRect.Y() - Scrollbar::PixelsPerLineStep());
        containerViewportRect.SetHeight(containerViewportRect.Height() + Scrollbar::PixelsPerLineStep());
        break;
      case dom::FocusDirection::Down:
        containerViewportRect.SetHeight(containerViewportRect.Height() + Scrollbar::PixelsPerLineStep());
        break;
      default: break;
    }

    auto *render = node.Renderer();
    if (!render)
    {
      return true;
    }

    LayoutRect rect(render->AbsoluteClippedOverflowRectForSpatialNavigation());
    if (rect.IsEmpty())
    {
      return true;
    }

    return !containerViewportRect.Intersects(rect);
  }

  bool ScrollInDirection(LocalFrame *frame, dom::FocusDirection direction) noexcept
  {
    krys_debug_assert(frame);

    if (frame && CanScrollInDirection(*frame->Document(), direction))
    {
      LayoutUnit dx;
      LayoutUnit dy;
      switch (direction)
      {
        case dom::FocusDirection::Left:  dx = -Scrollbar::PixelsPerLineStep(); break;
        case dom::FocusDirection::Right: dx = Scrollbar::PixelsPerLineStep(); break;
        case dom::FocusDirection::Up:    dy = -Scrollbar::PixelsPerLineStep(); break;
        case dom::FocusDirection::Down:  dy = Scrollbar::PixelsPerLineStep(); break;
        default:                         krys_debug_assert(false); return false;
      }

      frame->View()->ScrollBy(IntSize(dx.ToInt(), dy.ToInt()));
      return true;
    }
    return false;
  }

  bool ScrollInDirection(const dom::ContainerNode &container, dom::FocusDirection direction) noexcept
  {
    if (auto *document = DynamicDowncast<dom::Document>(container))
    {
      return ScrollInDirection(document->Frame(), direction);
    }

    if (!CanScrollInDirection(container, direction))
    {
      return false;
    }

    if (auto *renderBox = container.GetRenderBox())
    {
      LayoutUnit dx;
      LayoutUnit dy;
      switch (direction)
      {
        case dom::FocusDirection::Left:
          dx = -std::min<LayoutUnit>(Scrollbar::PixelsPerLineStep(), renderBox->ScrollLeft());
          break;
        case dom::FocusDirection::Right:
          krys_debug_assert(renderBox->ScrollWidth() > (renderBox->ScrollLeft() + renderBox->ClientWidth()));
          dx = std::min<LayoutUnit>(Scrollbar::PixelsPerLineStep(),
                                    renderBox->ScrollWidth()
                                      - (renderBox->ScrollLeft() + renderBox->ClientWidth()));
          break;
        case dom::FocusDirection::Up:
          dy = -std::min<LayoutUnit>(Scrollbar::PixelsPerLineStep(), renderBox->ScrollTop());
          break;
        case dom::FocusDirection::Down:
          krys_debug_assert(renderBox->ScrollHeight() - (renderBox->ScrollTop() + renderBox->ClientHeight()));
          dy = std::min<LayoutUnit>(Scrollbar::PixelsPerLineStep(),
                                    renderBox->ScrollHeight()
                                      - (renderBox->ScrollTop() + renderBox->ClientHeight()));
          break;
        default: krys_debug_assert(false); return false;
      }

      if (auto *scrollableArea = renderBox->EnclosingLayer()->GetScrollableArea())
      {
        scrollableArea->ScrollByRecursively(IntSize(dx.ToInt(), dy.ToInt()));
      }
      return true;
    }

    return false;
  }

  dom::ContainerNode *
    ScrollableEnclosingBoxOrParentFrameForNodeInDirection(dom::FocusDirection direction,
                                                          dom::ContainerNode &container) noexcept
  {
    // A document has no parent frame to go on to: there's one frame.
    dom::ContainerNode *parent = &container;
    do
    {
      parent = Is<dom::Document>(*parent) ? nullptr : parent->ParentNode();
    } while (parent && !CanScrollInDirection(*parent, direction) && !Is<dom::Document>(*parent));

    return parent;
  }

  bool CanScrollInDirection(const dom::ContainerNode &container, dom::FocusDirection direction) noexcept
  {
    if (Is<html::HTMLSelectElement>(container))
    {
      return false;
    }

    if (auto *document = DynamicDowncast<dom::Document>(container))
    {
      return CanScrollInDirection(document->Frame(), direction);
    }

    if (!IsScrollableNode(container))
    {
      return false;
    }

    if (auto *renderBox = container.GetRenderBox())
    {
      switch (direction)
      {
        case dom::FocusDirection::Left:
          return renderBox->Style().OverflowX() != Overflow::Hidden && renderBox->ScrollLeft() > 0;
        case dom::FocusDirection::Up:
          return renderBox->Style().OverflowY() != Overflow::Hidden && renderBox->ScrollTop() > 0;
        case dom::FocusDirection::Right:
          return renderBox->Style().OverflowX() != Overflow::Hidden
                 && renderBox->ScrollLeft() + renderBox->ClientWidth() < renderBox->ScrollWidth();
        case dom::FocusDirection::Down:
          return renderBox->Style().OverflowY() != Overflow::Hidden
                 && renderBox->ScrollTop() + renderBox->ClientHeight() < renderBox->ScrollHeight();
        default: krys_debug_assert(false); return false;
      }
    }
    krys_debug_assert(false);
    return false;
  }

  bool CanScrollInDirection(const LocalFrame *frame, dom::FocusDirection direction) noexcept
  {
    if (!frame || !frame->View())
    {
      return false;
    }
    ScrollbarMode verticalMode;
    ScrollbarMode horizontalMode;
    frame->View()->CalculateScrollbarModesForLayout(horizontalMode, verticalMode);
    if ((direction == dom::FocusDirection::Left || direction == dom::FocusDirection::Right)
        && ScrollbarMode::AlwaysOff == horizontalMode)
    {
      return false;
    }
    if ((direction == dom::FocusDirection::Up || direction == dom::FocusDirection::Down)
        && ScrollbarMode::AlwaysOff == verticalMode)
    {
      return false;
    }
    LayoutSize size(frame->View()->TotalContentsSize());
    LayoutPoint scrollPosition(frame->View()->GetScrollPosition());
    LayoutRect rect(
      frame->View()->UnobscuredContentRect(ScrollableArea::VisibleContentRectIncludesScrollbars::Yes));

    // FIXME: wrong in RTL documents.
    switch (direction)
    {
      case dom::FocusDirection::Left:  return scrollPosition.X() > 0;
      case dom::FocusDirection::Up:    return scrollPosition.Y() > 0;
      case dom::FocusDirection::Right: return rect.Width() + scrollPosition.X() < size.Width();
      case dom::FocusDirection::Down:  return rect.Height() + scrollPosition.Y() < size.Height();
      default:                         krys_debug_assert(false); return false;
    }
  }

  LayoutRect NodeRectInAbsoluteCoordinates(const dom::ContainerNode &containerNode,
                                           bool ignoreBorder) noexcept
  {
    krys_debug_assert(containerNode.Renderer()
                      && !containerNode.NodeDocument().View()->LayoutContext().NeedsLayout());

    if (auto *document = DynamicDowncast<dom::Document>(containerNode))
    {
      return FrameRectInAbsoluteCoordinates(document->Frame());
    }

    if (auto *renderer = containerNode.Renderer())
    {
      LayoutRect rect(renderer->AbsoluteBoundingBoxRect());
      // For authors that use border instead of outline in their CSS, we compensate by ignoring the border
      // when calculating the rect of the focused element.
      if (ignoreBorder)
      {
        const auto &style = renderer->Style();
        auto left = style::Evaluate<LayoutUnit>(style.BorderLeftWidth(), style::ZoomNeeded {});
        auto top = style::Evaluate<LayoutUnit>(style.BorderTopWidth(), style::ZoomNeeded {});
        auto right = style::Evaluate<LayoutUnit>(style.BorderRightWidth(), style::ZoomNeeded {});
        auto bottom = style::Evaluate<LayoutUnit>(style.BorderBottomWidth(), style::ZoomNeeded {});
        rect.Move(left, top);
        rect.SetWidth(rect.Width() - left - right);
        rect.SetHeight(rect.Height() - top - bottom);
      }
      return rect;
    }

    return {};
  }

  LayoutRect FrameRectInAbsoluteCoordinates(LocalFrame *frame) noexcept
  {
    return LayoutRect(frame->View()->VisibleContentRect());
  }

  bool AreElementsOnSameLine(const FocusCandidate &firstCandidate,
                             const FocusCandidate &secondCandidate) noexcept
  {
    if (firstCandidate.IsNull() || secondCandidate.IsNull())
    {
      return false;
    }

    if (!firstCandidate.VisibleNode->Renderer() || !secondCandidate.VisibleNode->Renderer())
    {
      return false;
    }

    if (!firstCandidate.Rect.Intersects(secondCandidate.Rect))
    {
      return false;
    }

    if (Is<html::HTMLAreaElement>(firstCandidate.FocusableNode.get())
        || Is<html::HTMLAreaElement>(secondCandidate.FocusableNode.get()))
    {
      return false;
    }

    if (!firstCandidate.VisibleNode->Renderer()->IsRenderInline()
        || !secondCandidate.VisibleNode->Renderer()->IsRenderInline())
    {
      return false;
    }

    if (firstCandidate.VisibleNode->Renderer()->ContainingBlock()
        != secondCandidate.VisibleNode->Renderer()->ContainingBlock())
    {
      return false;
    }

    return true;
  }

  // Consider only those nodes as candidate which are exactly in the focus-direction.
  // e.g. If we are moving down then the nodes that are above current focused node should be considered as
  // invalid.
  bool IsValidCandidate(dom::FocusDirection direction, const FocusCandidate &current,
                        FocusCandidate &candidate) noexcept
  {
    LayoutRect currentRect = current.Rect;
    LayoutRect candidateRect = candidate.Rect;

    switch (direction)
    {
      case dom::FocusDirection::Left:  return candidateRect.X() < currentRect.MaxX();
      case dom::FocusDirection::Up:    return candidateRect.Y() < currentRect.MaxY();
      case dom::FocusDirection::Right: return candidateRect.MaxX() > currentRect.X();
      case dom::FocusDirection::Down:  return candidateRect.MaxY() > currentRect.Y();
      default:                         krys_debug_assert(false);
    }
    return false;
  }

  void DistanceDataForNode(dom::FocusDirection direction, const FocusCandidate &current,
                           FocusCandidate &candidate) noexcept
  {
    if (AreElementsOnSameLine(current, candidate))
    {
      if ((direction == dom::FocusDirection::Up && current.Rect.Y() > candidate.Rect.Y())
          || (direction == dom::FocusDirection::Down && candidate.Rect.Y() > current.Rect.Y()))
      {
        candidate.Distance = 0;
        candidate.Alignment = RectsAlignment::Full;
        return;
      }
    }

    LayoutRect nodeRect = candidate.Rect;
    LayoutRect currentRect = current.Rect;
    DeflateIfOverlapped(currentRect, nodeRect);

    if (!IsRectInDirection(direction, currentRect, nodeRect))
    {
      return;
    }

    LayoutPoint exitPoint;
    LayoutPoint entryPoint;
    LayoutUnit sameAxisDistance;
    LayoutUnit otherAxisDistance;
    EntryAndExitPointsForDirection(direction, currentRect, nodeRect, exitPoint, entryPoint);

    switch (direction)
    {
      case dom::FocusDirection::Left:
        sameAxisDistance = exitPoint.X() - entryPoint.X();
        otherAxisDistance = AbsoluteValue(exitPoint.Y() - entryPoint.Y());
        break;
      case dom::FocusDirection::Up:
        sameAxisDistance = exitPoint.Y() - entryPoint.Y();
        otherAxisDistance = AbsoluteValue(exitPoint.X() - entryPoint.X());
        break;
      case dom::FocusDirection::Right:
        sameAxisDistance = entryPoint.X() - exitPoint.X();
        otherAxisDistance = AbsoluteValue(entryPoint.Y() - exitPoint.Y());
        break;
      case dom::FocusDirection::Down:
        sameAxisDistance = entryPoint.Y() - exitPoint.Y();
        otherAxisDistance = AbsoluteValue(entryPoint.X() - exitPoint.X());
        break;
      default: krys_debug_assert(false); return;
    }

    float x = ((entryPoint.X() - exitPoint.X()) * (entryPoint.X() - exitPoint.X())).ToFloat();
    float y = ((entryPoint.Y() - exitPoint.Y()) * (entryPoint.Y() - exitPoint.Y())).ToFloat();

    float euclidianDistance = std::sqrt(x + y);

    // Loosely based on http://www.w3.org/TR/WICD/#focus-handling
    // df = dotDist + dx + dy + 2 * (xdisplacement + ydisplacement) - sqrt(Overlap)

    float distance = euclidianDistance + sameAxisDistance.ToFloat() + 2 * otherAxisDistance.ToFloat();
    candidate.Distance = static_cast<int64>(std::roundf(distance));
    // The main frame's view: there's one frame.
    auto *view = candidate.VisibleNode->NodeDocument().View();
    if (!view)
    {
      return;
    }
    LayoutSize viewSize(view->VisibleContentRect().Size());
    candidate.Alignment = AlignmentForRects(direction, currentRect, nodeRect, viewSize);
  }

  bool CanBeScrolledIntoView(dom::FocusDirection direction, const FocusCandidate &candidate) noexcept
  {
    krys_debug_assert(candidate.VisibleNode && candidate.IsOffscreen);
    LayoutRect candidateRect = candidate.Rect;
    for (dom::ContainerNode *parentNode = candidate.VisibleNode->ParentNode(); parentNode;
         parentNode = parentNode->ParentNode())
    {
      if (!parentNode->Renderer())
      {
        continue;
      }
      LayoutRect parentRect = NodeRectInAbsoluteCoordinates(*parentNode);
      if (!candidateRect.Intersects(parentRect))
      {
        if (((direction == dom::FocusDirection::Left || direction == dom::FocusDirection::Right)
             && parentNode->Renderer()->Style().OverflowX() == Overflow::Hidden)
            || ((direction == dom::FocusDirection::Up || direction == dom::FocusDirection::Down)
                && parentNode->Renderer()->Style().OverflowY() == Overflow::Hidden))
        {
          return false;
        }
      }
      if (parentNode == candidate.EnclosingScrollableBox.get())
      {
        return CanScrollInDirection(*parentNode, direction);
      }
    }
    return true;
  }

  // The starting rect is the rect of the focused node, in document coordinates.
  // Compose a virtual starting rect if there is no focused node or if it is off screen.
  // The virtual rect is the edge of the container or frame. We select which
  // edge depending on the direction of the navigation.
  LayoutRect VirtualRectForDirection(dom::FocusDirection direction, const LayoutRect &startingRect,
                                     LayoutUnit width) noexcept
  {
    LayoutRect virtualStartingRect = startingRect;
    switch (direction)
    {
      case dom::FocusDirection::Left:
        virtualStartingRect.SetX(virtualStartingRect.MaxX() - width);
        virtualStartingRect.SetWidth(width);
        break;
      case dom::FocusDirection::Up:
        virtualStartingRect.SetY(virtualStartingRect.MaxY() - width);
        virtualStartingRect.SetHeight(width);
        break;
      case dom::FocusDirection::Right: virtualStartingRect.SetWidth(width); break;
      case dom::FocusDirection::Down:  virtualStartingRect.SetHeight(width); break;
      default:                         krys_debug_assert(false);
    }

    return virtualStartingRect;
  }

  LayoutRect VirtualRectForAreaElementAndDirection(html::HTMLAreaElement *area,
                                                   dom::FocusDirection direction) noexcept
  {
    krys_debug_assert(area);
    krys_debug_assert(area->ImageElement());
    // Area elements tend to overlap more than other focusable elements. We flatten the rect of the area
    // elements to minimize the effect of overlapping areas.
    LayoutRect rect =
      VirtualRectForDirection(direction, area->ComputeRect(area->ImageElement()->Renderer()), 1);
    return rect;
  }
}
