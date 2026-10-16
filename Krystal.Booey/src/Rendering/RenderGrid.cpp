#include "Krystal.Booey/Rendering/RenderGrid.hpp"

namespace krys::boo
{
  RenderGrid::RenderGrid(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlock(Type::Grid, element, krys::move(style), {})
  {
    krys_debug_assert(IsRenderGrid());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderGrid::~RenderGrid() noexcept = default;

  utf8_stringview RenderGrid::RenderName() const noexcept
  {
    if (IsFloating())
    {
      return u8"RenderGrid (floating)";
    }
    if (IsOutOfFlowPositioned())
    {
      return u8"RenderGrid (positioned)";
    }
    if (IsAnonymous())
    {
      return u8"RenderGrid (generated)";
    }
    if (IsRelativelyPositioned())
    {
      return u8"RenderGrid (relative positioned)";
    }
    return u8"RenderGrid";
  }

  const List<LayoutUnit> &RenderGrid::Positions(style::GridTrackSizingDirection) const noexcept
  {
    krys_unreachable();
  }

  bool RenderGrid::IsSubgrid(style::GridTrackSizingDirection) const noexcept
  {
    krys_unreachable();
  }

  uint32 RenderGrid::NumTracks(style::GridTrackSizingDirection) const noexcept
  {
    krys_unreachable();
  }

  uint32 RenderGrid::AutoRepeatCountForDirection(style::GridTrackSizingDirection) const noexcept
  {
    krys_unreachable();
  }

  List<LayoutUnit> RenderGrid::TrackSizesForComputedStyle(style::GridTrackSizingDirection) const noexcept
  {
    krys_unreachable();
  }

  int32 RenderGrid::ExplicitGridStartForDirection(style::GridTrackSizingDirection) const noexcept
  {
    krys_unreachable();
  }
}
