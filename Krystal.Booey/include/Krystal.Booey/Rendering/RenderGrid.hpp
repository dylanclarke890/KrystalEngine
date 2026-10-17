#pragma once

#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizingDirection.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  /// @brief A grid container.
  // FIXME: Grid layout (the grid, track sizing, masonry) comes with layout.
  class RenderGrid : public RenderBlock
  {
    KRYS_NON_COPY_MOVABLE(RenderGrid);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderGrid);

  private:
    mutable bool _isComputingTrackSizes {false};

  public:
    RenderGrid(dom::Element &element, RenderStyle &&style) noexcept;
    ~RenderGrid() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override;

    // FIXME: Layout queries, reachable only through `RendererOf` and `RenderBoxOf`, which return null until
    // there's layout.
    KRYS_NODISCARD const List<LayoutUnit> &Positions(style::GridTrackSizingDirection) const noexcept;

    KRYS_NODISCARD bool IsMasonry() const noexcept;
    KRYS_NODISCARD bool IsMasonry(style::GridTrackSizingDirection direction) const noexcept;
    KRYS_NODISCARD bool IsExtrinsicallySized() const noexcept;
    KRYS_NODISCARD bool
      IsSubgridInParentDirection(style::GridTrackSizingDirection parentDirection) const noexcept;

    KRYS_NODISCARD bool IsComputingTrackSizes() const noexcept
    {
      return _isComputingTrackSizes;
    }

    KRYS_NODISCARD ItemPosition
      SelfAlignmentNormalBehavior(const RenderBox *gridItem = nullptr) const noexcept override
    {
      krys_debug_assert(gridItem);
      return gridItem->IsRenderReplaced() ? ItemPosition::Start : ItemPosition::Stretch;
    }
    KRYS_NODISCARD bool IsSubgrid(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD uint32 NumTracks(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD uint32 AutoRepeatCountForDirection(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD List<LayoutUnit>
      TrackSizesForComputedStyle(style::GridTrackSizingDirection) const noexcept;
    KRYS_NODISCARD int32 ExplicitGridStartForDirection(style::GridTrackSizingDirection) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderGrid)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderGrid();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
