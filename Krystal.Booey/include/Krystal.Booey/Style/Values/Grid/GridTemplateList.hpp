#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridNamedLinesMap.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridOrderedNamedLinesMap.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSize.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::style
{
  using RepeatEntry = Variant<GridTrackSize, List<css::CSSOMString>>;
  using RepeatTrackList = List<RepeatEntry>;

  struct GridTrackEntryRepeat
  {
    uint32 Repeats {0};
    RepeatTrackList Tracks {};

    KRYS_NODISCARD bool operator==(const GridTrackEntryRepeat &) const noexcept = default;
  };

  struct GridTrackEntryAutoRepeat
  {
    AutoRepeatType Type {AutoRepeatType::None};
    RepeatTrackList Tracks {};

    KRYS_NODISCARD bool operator==(const GridTrackEntryAutoRepeat &) const noexcept = default;
  };

  struct GridTrackEntrySubgrid
  {
    KRYS_NODISCARD bool operator==(const GridTrackEntrySubgrid &) const noexcept = default;
  };

  using GridTrackEntry = Variant<GridTrackSize, List<css::CSSOMString>, GridTrackEntryRepeat,
                                 GridTrackEntryAutoRepeat, GridTrackEntrySubgrid>;
  using GridTrackList = List<GridTrackEntry>;

  // <'grid-template-columns'/'grid-template-rows'> = none | <track-list> | <auto-track-list>
  struct GridTemplateList
  {
    GridTrackList Tracks {};

    // Calculated from `Tracks`.

    List<GridTrackSize> Sizes {};
    GridNamedLinesMap NamedLines {};
    GridOrderedNamedLinesMap OrderedNamedLines {};

    List<GridTrackSize> AutoRepeatSizes {};
    GridNamedLinesMap AutoRepeatNamedLines {};
    GridOrderedNamedLinesMap AutoRepeatOrderedNamedLines {};

    uint32 AutoRepeatInsertionPoint {0};
    boo::AutoRepeatType AutoRepeatType {boo::AutoRepeatType::None};

    bool Subgrid {false};

    GridTemplateList(css::keywords::None) noexcept
    {
    }
    GridTemplateList(GridTrackList &&entries) noexcept;

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return Tracks.empty();
    }

    KRYS_NODISCARD bool operator==(const GridTemplateList &other) const noexcept
    {
      // It is only necessary to compare the `Tracks` member, as the other members
      // are purely cached derived values based on the value of `Tracks`.
      return Tracks == other.Tracks;
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<GridTemplateList>
  {
    KRYS_NODISCARD GridTemplateList operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<GridTemplateList>
  {
    KRYS_NODISCARD bool CanBlend(const GridTemplateList &, const GridTemplateList &) noexcept;
    KRYS_NODISCARD GridTemplateList Blend(const GridTemplateList &, const GridTemplateList &,
                                          const BlendingContext &) noexcept;
  };
}
