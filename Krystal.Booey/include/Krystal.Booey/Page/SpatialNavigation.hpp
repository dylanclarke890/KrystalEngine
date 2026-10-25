#pragma once

#include "Krystal.Booey/DOM/FocusOptions.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <limits>

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
  class Node;
}

namespace krys::boo::html
{
  class HTMLAreaElement;
}

namespace krys::boo
{

  class LocalFrame;

  KRYS_NODISCARD constexpr inline int64 MaxDistance() noexcept
  {
    return std::numeric_limits<int64>::max();
  }

  KRYS_NODISCARD constexpr inline int32 FudgeFactor() noexcept
  {
    return 2;
  }

  // Spatially speaking, two given elements in a web page can be:
  // 1) Fully aligned: There is a full intersection between the rects, either
  //    vertically or horizontally.
  //
  // * Horizontally       * Vertically
  //    _
  //   |_|                   _ _ _ _ _ _
  //   |_|...... _          |_|_|_|_|_|_|
  //   |_|      |_|         .       .
  //   |_|......|_|   OR    .       .
  //   |_|      |_|         .       .
  //   |_|......|_|          _ _ _ _
  //   |_|                  |_|_|_|_|
  //
  //
  // 2) Partially aligned: There is a partial intersection between the rects, either
  //    vertically or horizontally.
  //
  // * Horizontally       * Vertically
  //    _                   _ _ _ _ _
  //   |_|                 |_|_|_|_|_|
  //   |_|.... _      OR           . .
  //   |_|    |_|                  . .
  //   |_|....|_|                  ._._ _
  //          |_|                  |_|_|_|
  //          |_|
  //
  // 3) Or, otherwise, not aligned at all.
  //
  // * Horizontally       * Vertically
  //         _              _ _ _ _
  //        |_|            |_|_|_|_|
  //        |_|                    .
  //        |_|                     .
  //       .          OR             .
  //    _ .                           ._ _ _ _ _
  //   |_|                            |_|_|_|_|_|
  //   |_|
  //   |_|
  //
  // "Totally Aligned" elements are preferable candidates to move
  // focus to over "Partially Aligned" ones, that on its turns are
  // more preferable than "Not Aligned".
  enum class RectsAlignment : uint8
  {
    None = 0,
    Partial,
    Full
  };

  /// @brief An element that directional focus navigation could move to, with its rect and how it lies from
  /// the starting rect.
  struct FocusCandidate
  {
    // We handle differently visibleNode and FocusableNode to properly handle the areas of imagemaps,
    // where visibleNode would represent the image element and focusableNode would represent the area element.
    // In all other cases, visibleNode and focusableNode are one and the same.
    WeakPtr<dom::Element> VisibleNode;
    WeakPtr<dom::Element> FocusableNode;
    WeakPtr<dom::ContainerNode> EnclosingScrollableBox;
    int64 Distance {MaxDistance()};
    RectsAlignment Alignment {RectsAlignment::None};
    LayoutRect Rect;
    bool IsOffscreen {false};
    bool IsOffscreenAfterScrolling {true};

    FocusCandidate() noexcept = default;
    FocusCandidate(dom::Element *element, dom::FocusDirection direction) noexcept;

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return !VisibleNode;
    }

    KRYS_NODISCARD bool InScrollableContainer() const noexcept
    {
      return VisibleNode && EnclosingScrollableBox;
    }

    KRYS_NODISCARD dom::Document *Document() const noexcept;
  };

  /// @brief Whether @p node is outside the viewport of its document (after scrolling a line in @p direction,
  /// if there is one).
  KRYS_NODISCARD bool HasOffscreenRect(const dom::Node &node,
                                       dom::FocusDirection direction = dom::FocusDirection::None) noexcept;
  bool ScrollInDirection(LocalFrame *frame, dom::FocusDirection direction) noexcept;
  bool ScrollInDirection(const dom::ContainerNode &container, dom::FocusDirection direction) noexcept;
  KRYS_NODISCARD bool CanScrollInDirection(const dom::ContainerNode &container,
                                           dom::FocusDirection direction) noexcept;
  KRYS_NODISCARD bool CanScrollInDirection(const LocalFrame *frame, dom::FocusDirection direction) noexcept;
  KRYS_NODISCARD bool CanBeScrolledIntoView(dom::FocusDirection direction,
                                            const FocusCandidate &candidate) noexcept;
  KRYS_NODISCARD bool AreElementsOnSameLine(const FocusCandidate &firstCandidate,
                                            const FocusCandidate &secondCandidate) noexcept;
  KRYS_NODISCARD bool IsValidCandidate(dom::FocusDirection direction, const FocusCandidate &current,
                                       FocusCandidate &candidate) noexcept;
  void DistanceDataForNode(dom::FocusDirection direction, const FocusCandidate &current,
                           FocusCandidate &candidate) noexcept;
  KRYS_NODISCARD dom::ContainerNode *
    ScrollableEnclosingBoxOrParentFrameForNodeInDirection(dom::FocusDirection direction,
                                                          dom::ContainerNode &container) noexcept;
  KRYS_NODISCARD LayoutRect NodeRectInAbsoluteCoordinates(const dom::ContainerNode &containerNode,
                                                          bool ignoreBorder = false) noexcept;
  KRYS_NODISCARD LayoutRect FrameRectInAbsoluteCoordinates(LocalFrame *frame) noexcept;
  KRYS_NODISCARD LayoutRect VirtualRectForDirection(dom::FocusDirection direction,
                                                    const LayoutRect &startingRect,
                                                    LayoutUnit width = LayoutUnit()) noexcept;
  KRYS_NODISCARD LayoutRect VirtualRectForAreaElementAndDirection(html::HTMLAreaElement *area,
                                                                  dom::FocusDirection direction) noexcept;
}
