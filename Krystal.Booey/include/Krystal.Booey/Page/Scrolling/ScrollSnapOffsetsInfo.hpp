#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutSize.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollTypes.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <utility>

namespace krys::boo
{
  namespace dom
  {
    class Element;
  }
  class RenderBox;
  class RenderStyle;
  class ScrollableArea;
  class WritingMode;

  /// @brief A scroll offset that a scroll container can snap to on one axis.
  struct SnapOffset
  {
    LayoutUnit Offset;
    ScrollSnapStop Stop {ScrollSnapStop::Normal};
    bool HasSnapAreaLargerThanViewport {false};
    /// @brief The snap areas (in `ScrollSnapOffsetsInfo::SnapAreas`) that align at this offset.
    List<uint32> SnapAreaIndices;
  };

  /// @brief The snap positions of a scroll container.
  /// @see https://drafts.csswg.org/css-scroll-snap-1/#snap-position
  struct ScrollSnapOffsetsInfo
  {
    ScrollSnapStrictness Strictness {ScrollSnapStrictness::Proximity};
    List<SnapOffset> HorizontalSnapOffsets;
    List<SnapOffset> VerticalSnapOffsets;
    /// @brief The snap areas, relative to the scroll offsets.
    List<LayoutRect> SnapAreas;
    /// @brief The element of each of `SnapAreas`.
    List<WeakPtr<dom::Element>> SnapAreaTargets;

    KRYS_NODISCARD const List<SnapOffset> &OffsetsForAxis(ScrollEventAxis axis) const noexcept
    {
      return axis == ScrollEventAxis::Vertical ? VerticalSnapOffsets : HorizontalSnapOffsets;
    }

    /// @brief The offset on @p axis to snap to, for a scroll to @p destination, and which of the axis's snap
    /// offsets that is: none if it doesn't snap.
    /// @param velocity The scroll's direction on @p axis: 0 for the closest offset.
    /// @param originalOffset Where a directional scroll started on @p axis, which it doesn't snap back to or
    /// before.
    /// @see https://drafts.csswg.org/css-scroll-snap-1/#choosing
    KRYS_NODISCARD std::pair<LayoutUnit, Maybe<uint32>>
      ClosestSnapOffset(ScrollEventAxis axis, const LayoutSize &viewportSize, LayoutPoint destination,
                        float velocity, Maybe<LayoutUnit> originalOffset = null) const noexcept;
  };

  /// @brief Gives @p scrollableArea the snap offsets of the boxes that snap in @p scrollingElementBox, or
  /// none if it isn't a scroll snap container.
  /// @param viewportRect The optimal viewing region's starting point, in border box coordinates.
  /// @see https://drafts.csswg.org/css-scroll-snap-1/#scroll-snap-type
  void UpdateSnapOffsetsForScrollableArea(ScrollableArea &scrollableArea,
                                          const RenderBox &scrollingElementBox,
                                          const LayoutRect &viewportRect) noexcept;
}
