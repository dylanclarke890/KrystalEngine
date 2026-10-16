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

  public:
    RenderGrid(dom::Element &element, RenderStyle &&style) noexcept;
    ~RenderGrid() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override;

    // FIXME: Layout queries, reachable only through `RendererOf` and `RenderBoxOf`, which return null until
    // there's layout.
    KRYS_NODISCARD const List<LayoutUnit> &Positions(style::GridTrackSizingDirection) const noexcept;
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
