#pragma once

#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderBox;
  class RenderGrid;

  /// @brief Walks a grid item's ancestor subgrids in one direction, from its parent outwards, while each one
  /// is subgridded in that direction.
  class AncestorSubgridIterator
  {
  private:
    const RenderGrid *_firstAncestorSubgrid {nullptr};
    RenderGrid *_currentAncestorSubgrid {nullptr};
    Maybe<style::GridTrackSizingDirection> _direction;

  public:
    AncestorSubgridIterator() noexcept = default;
    AncestorSubgridIterator(const RenderGrid *firstAncestorSubgrid,
                            style::GridTrackSizingDirection direction) noexcept;

    KRYS_NODISCARD RenderGrid &operator*() const noexcept;

    KRYS_NODISCARD bool operator==(const AncestorSubgridIterator &other) const noexcept;

    AncestorSubgridIterator &operator++() noexcept;
    KRYS_NODISCARD AncestorSubgridIterator begin() const noexcept;
    KRYS_NODISCARD AncestorSubgridIterator end() const noexcept;

  private:
    AncestorSubgridIterator(const RenderGrid *firstAncestorSubgrid, RenderGrid *currentAncestor,
                            Maybe<style::GridTrackSizingDirection> direction) noexcept;
  };

  KRYS_NODISCARD AncestorSubgridIterator
    AncestorSubgridsOfGridItem(const RenderBox &gridItem, style::GridTrackSizingDirection direction) noexcept;
}
