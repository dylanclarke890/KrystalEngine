#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  class GridSpan;
  class RenderBox;
  class RenderGrid;
}

namespace krys::boo::style
{
  enum class GridPositionSide : uint8;
  enum class GridTrackSizingDirection : bool;

  /// @brief Resolves grid items' positions from their style.
  class GridPositionsResolver
  {
  public:
    KRYS_NODISCARD static GridPositionSide InitialPositionSide(GridTrackSizingDirection direction) noexcept;
    KRYS_NODISCARD static GridPositionSide FinalPositionSide(GridTrackSizingDirection direction) noexcept;
    KRYS_NODISCARD static uint32 SpanSizeForAutoPlacedItem(const RenderBox &gridItem,
                                                           GridTrackSizingDirection direction) noexcept;
    KRYS_NODISCARD static GridSpan ResolveGridPositionsFromStyle(const RenderGrid &gridContainer,
                                                                 const RenderBox &gridItem,
                                                                 GridTrackSizingDirection direction) noexcept;
    KRYS_NODISCARD static uint32 ExplicitGridCount(const RenderGrid &gridContainer,
                                                   GridTrackSizingDirection direction) noexcept;
  };
}
