#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/Editing/EditingBoundary.hpp"
#include "Krystal.Booey/Editing/InlineRunAndOffset.hpp"
#include "Krystal.Booey/Editing/TextAffinity.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <compare>

namespace krys::boo
{
  class RenderObject;

  /// @brief A position where a caret can go: a candidate position canonicalised among the positions that
  /// render in the same place, with the affinity that tells which line it's on when it's at a line wrap.
  class VisiblePosition
  {
  private:
    dom::Position _deepPosition;
    Affinity _affinity {DefaultAffinity};

  public:
    // VisiblePosition default affinity is downstream for callers that do not really care because it is more
    // efficient than upstream.
    static constexpr auto DefaultAffinity = Affinity::Downstream;

    VisiblePosition() noexcept = default;

    // This constructor will ignore the passed-in affinity if the position is not at the end of a line.
    VisiblePosition(const dom::Position &position, Affinity affinity = DefaultAffinity) noexcept;

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return _deepPosition.IsNull();
    }
    KRYS_NODISCARD bool IsNotNull() const noexcept
    {
      return _deepPosition.IsNotNull();
    }
    KRYS_NODISCARD bool IsOrphan() const noexcept
    {
      return _deepPosition.IsOrphan();
    }

    KRYS_NODISCARD dom::Position DeepEquivalent() const noexcept
    {
      return _deepPosition;
    }
    KRYS_NODISCARD Affinity GetAffinity() const noexcept
    {
      return _affinity;
    }
    void SetAffinity(Affinity affinity) noexcept
    {
      _affinity = affinity;
    }

    // FIXME: Change the following functions' parameter from a boolean to StayInEditableContent.

    // Next() and Previous() increment/decrement by a character cluster.
    KRYS_NODISCARD VisiblePosition
      Next(EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CanCrossEditingBoundary,
           bool *reachedBoundary = nullptr) const noexcept;
    KRYS_NODISCARD VisiblePosition
      Previous(EditingBoundaryCrossingRule rule = EditingBoundaryCrossingRule::CanCrossEditingBoundary,
               bool *reachedBoundary = nullptr) const noexcept;
    KRYS_NODISCARD VisiblePosition HonorEditingBoundaryAtOrBefore(
      const VisiblePosition &position, bool *reachedBoundary = nullptr) const noexcept;
    KRYS_NODISCARD VisiblePosition HonorEditingBoundaryAtOrAfter(
      const VisiblePosition &position, bool *reachedBoundary = nullptr) const noexcept;

    KRYS_NODISCARD VisiblePosition Left(bool stayInEditableContent = false,
                                        bool *reachedBoundary = nullptr) const noexcept;
    KRYS_NODISCARD VisiblePosition Right(bool stayInEditableContent = false,
                                         bool *reachedBoundary = nullptr) const noexcept;

    KRYS_NODISCARD char32 CharacterAfter() const noexcept;
    KRYS_NODISCARD char32 CharacterBefore() const noexcept
    {
      return Previous().CharacterAfter();
    }

    // FIXME: This does not handle [table, 0] correctly.
    KRYS_NODISCARD dom::Element *RootEditableElement() const noexcept
    {
      return _deepPosition.IsNotNull() ? _deepPosition.DeprecatedNode()->RootEditableElement() : nullptr;
    }

    KRYS_NODISCARD InlineBoxAndOffset GetInlineBoxAndOffset() const noexcept;
    KRYS_NODISCARD InlineBoxAndOffset GetInlineBoxAndOffset(TextDirection primaryDirection) const noexcept;

    struct LocalCaretRectResult
    {
      LayoutRect Rect;
      RenderObject *Renderer {nullptr};
    };

    KRYS_NODISCARD LocalCaretRectResult LocalCaretRect() const noexcept;
    // Bounds of (possibly transformed) caret in absolute coords.
    KRYS_NODISCARD IntRect AbsoluteCaretBounds(bool *insideFixed = nullptr) const noexcept;
    // Abs x/y position of the caret ignoring transforms.
    // FIXME: navigation with transforms should be smarter.
    KRYS_NODISCARD int32 LineDirectionPointForBlockDirectionNavigation() const noexcept;

    KRYS_NODISCARD FloatRect AbsoluteSelectionBoundsForLine() const noexcept;

    // This is a tentative enhancement of operator== to account for affinity.
    // FIXME: Combine this function with operator==.
    KRYS_NODISCARD bool Equals(const VisiblePosition &other) const noexcept;

    // FIXME: Is it correct and helpful for this to be ignoring differences in affinity?
    KRYS_NODISCARD friend bool operator==(const VisiblePosition &a, const VisiblePosition &b) noexcept
    {
      return a.DeepEquivalent() == b.DeepEquivalent();
    }

  private:
    KRYS_NODISCARD static dom::Position CanonicalPosition(const dom::Position &position) noexcept;
    KRYS_NODISCARD dom::Position LeftVisuallyDistinctCandidate() const noexcept;
    KRYS_NODISCARD dom::Position RightVisuallyDistinctCandidate() const noexcept;
  };

  KRYS_NODISCARD std::partial_ordering operator<=>(const VisiblePosition &a,
                                                   const VisiblePosition &b) noexcept;

  KRYS_NODISCARD Maybe<dom::BoundaryPoint> MakeBoundaryPoint(const VisiblePosition &position) noexcept;

  KRYS_NODISCARD dom::Element *EnclosingBlockFlowElement(const VisiblePosition &visiblePosition) noexcept;

  KRYS_NODISCARD bool IsFirstVisiblePositionInNode(const VisiblePosition &visiblePosition,
                                                   const dom::Node *node) noexcept;
  KRYS_NODISCARD bool IsLastVisiblePositionInNode(const VisiblePosition &visiblePosition,
                                                  const dom::Node *node) noexcept;

  KRYS_NODISCARD bool AreVisiblePositionsInSameTreeScope(const VisiblePosition &a,
                                                         const VisiblePosition &b) noexcept;

  KRYS_NODISCARD dom::Node *CommonInclusiveAncestor(const VisiblePosition &a,
                                                    const VisiblePosition &b) noexcept;

  /// @brief A pair of visible positions, not necessarily in order.
  struct VisiblePositionRange
  {
    VisiblePosition Start;
    VisiblePosition End;

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return Start.IsNull() || End.IsNull();
    }
  };

  KRYS_NODISCARD Maybe<dom::SimpleRange> MakeSimpleRange(const VisiblePositionRange &range) noexcept;
  KRYS_NODISCARD VisiblePositionRange MakeVisiblePositionRange(const Maybe<dom::SimpleRange> &range) noexcept;

  KRYS_NODISCARD dom::Node *CommonInclusiveAncestor(const VisiblePositionRange &range) noexcept;
  KRYS_NODISCARD bool Intersects(const VisiblePositionRange &a, const VisiblePositionRange &b) noexcept;
  KRYS_NODISCARD bool Contains(const VisiblePositionRange &range, const VisiblePosition &point) noexcept;
  KRYS_NODISCARD VisiblePositionRange Intersection(const VisiblePositionRange &a,
                                                   const VisiblePositionRange &b) noexcept;
  KRYS_NODISCARD VisiblePosition Midpoint(const VisiblePositionRange &range) noexcept;
}
