#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Web/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Web/Html/Algorithms/FocusOptions.hpp"
#include <limits>

namespace krys::web
{
  class ContainerNode;
  class Document;
  class Element;
  class Frame;
  class HTMLAreaElement;
  class Node;

  /// @brief A candidate's distance before it's measured: further than any measured one.
  KRYS_NODISCARD constexpr inline int64 MaxDistance() noexcept
  {
    return std::numeric_limits<int64>::max();
  }

  /// @brief How a candidate's rect lines up with the starting rect, across the direction of navigation (their
  /// rows when moving left or right, their columns when moving up or down). A better alignment wins over a
  /// nearer candidate.
  enum class RectsAlignment : uint8
  {
    /// @brief They don't overlap across the direction, or they're more than a screen apart.
    None = 0,
    /// @brief They overlap, but not fully.
    Partial,
    /// @brief One's middle falls within the other, or they share a start or end edge.
    Full,
  };

  /// @brief An element that directional focus navigation could move to, with its rect and how it lies from
  /// the starting rect.
  struct FocusCandidate
  {
    /// @brief The element drawn: the image for an image map's area, otherwise the focusable element.
    WeakPtr<Element> VisibleNode;
    /// @brief The element that would get focus.
    WeakPtr<Element> FocusableNode;
    /// @brief The nearest box around it that can scroll in the direction of navigation, or its document.
    WeakPtr<ContainerNode> EnclosingScrollableBox;
    /// @brief How far it is from the starting rect, weighing distance across the direction more.
    int64 Distance {MaxDistance()};
    RectsAlignment Alignment {RectsAlignment::None};
    /// @brief Its rect in document coordinates, without borders.
    LayoutRect Rect;
    /// @brief Whether it's outside its document's viewport.
    bool IsOffscreen {false};
    /// @brief Whether it's still outside the viewport after scrolling a line in the direction of navigation.
    bool IsOffscreenAfterScrolling {true};

    FocusCandidate() noexcept = default;

    /// @brief @p element as a candidate for moving in @p direction; null if it isn't rendered.
    FocusCandidate(Element *element, html::FocusDirection direction) noexcept;

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return !VisibleNode;
    }
  };

  /// @brief Whether @p node is outside the viewport of its document (after scrolling a line in @p direction,
  /// if there is one).
  KRYS_NODISCARD bool HasOffscreenRect(const Node &node,
                                       html::FocusDirection direction = html::FocusDirection::None) noexcept;

  /// @brief Scrolls @p frame's view a line in @p direction.
  /// @return Whether it could scroll.
  bool ScrollInDirection(Frame *frame, html::FocusDirection direction) noexcept;

  /// @brief Scrolls @p container (a document's view, or a scrollable box) a line in @p direction.
  /// @return Whether it could scroll.
  bool ScrollInDirection(const ContainerNode &container, html::FocusDirection direction) noexcept;

  /// @brief Whether @p container has more to scroll to in @p direction.
  KRYS_NODISCARD bool CanScrollInDirection(const ContainerNode &container,
                                           html::FocusDirection direction) noexcept;

  /// @brief Whether @p frame's view has more to scroll to in @p direction.
  KRYS_NODISCARD bool CanScrollInDirection(const Frame *frame, html::FocusDirection direction) noexcept;

  /// @brief Whether an offscreen @p candidate could be scrolled into view in @p direction: no box up to its
  /// scrollable box clips it away in that direction, and that box can scroll.
  KRYS_NODISCARD bool CanBeScrolledIntoView(html::FocusDirection direction,
                                            const FocusCandidate &candidate) noexcept;

  /// @brief Whether both candidates are inline boxes (not areas) in the same containing block whose rects
  /// overlap: moving up or down between them is moving along a line.
  KRYS_NODISCARD bool AreElementsOnSameLine(const FocusCandidate &firstCandidate,
                                            const FocusCandidate &secondCandidate) noexcept;

  /// @brief Whether @p candidate lies at least partly in @p direction from @p current.
  KRYS_NODISCARD bool IsValidCandidate(html::FocusDirection direction, const FocusCandidate &current,
                                       FocusCandidate &candidate) noexcept;

  /// @brief Sets @p candidate's distance and alignment from @p current, if it lies in @p direction.
  void DistanceDataForNode(html::FocusDirection direction, const FocusCandidate &current,
                           FocusCandidate &candidate) noexcept;

  /// @brief The nearest ancestor of @p container that can scroll in @p direction, or its document; null for
  /// a document.
  KRYS_NODISCARD ContainerNode *
    ScrollableEnclosingBoxOrParentFrameForNodeInDirection(html::FocusDirection direction,
                                                          ContainerNode &container) noexcept;

  /// @brief @p containerNode's rect in document coordinates (a document's is its visible content rect),
  /// with or without its borders.
  KRYS_NODISCARD LayoutRect NodeRectInAbsoluteCoordinates(const ContainerNode &containerNode,
                                                          bool ignoreBorder = false) noexcept;

  /// @brief A strip @p width thick along the edge of @p startingRect that navigation in @p direction moves
  /// away from, to start from when nothing focused is on screen.
  KRYS_NODISCARD LayoutRect VirtualRectForDirection(html::FocusDirection direction,
                                                    const LayoutRect &startingRect,
                                                    LayoutUnit width = LayoutUnit()) noexcept;

  /// @brief An image map area's rect, flattened to a 1px strip along the edge that navigation in
  /// @p direction moves away from, so overlapping areas don't hide each other.
  KRYS_NODISCARD LayoutRect VirtualRectForAreaElementAndDirection(HTMLAreaElement *area,
                                                                  html::FocusDirection direction) noexcept;
}
