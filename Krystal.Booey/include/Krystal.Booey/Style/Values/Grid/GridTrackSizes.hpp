#pragma once

#include "Krystal.Booey/Style/Values/Grid/GridTrackSize.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  using GridTrackSizeList = css::SpaceSeparatedFixedList<GridTrackSize>;

  // Default values for <'grid-auto-columns'>/<'grid-auto-rows'> is 'auto'.
  struct GridTrackSizeDefaulter
  {
    KRYS_NODISCARD const GridTrackSize &operator()() const noexcept;
    KRYS_NODISCARD bool operator==(const GridTrackSizeDefaulter &) const noexcept = default;
  };

  // <'grid-auto-columns'>/<'grid-auto-rows'> = <track-size>+
  // https://www.w3.org/TR/css-grid-2/#propdef-grid-auto-rows
  struct GridTrackSizes : css::ListOrDefault<GridTrackSizeList, GridTrackSizeDefaulter>
  {
    using css::ListOrDefault<GridTrackSizeList, GridTrackSizeDefaulter>::ListOrDefault;

    // Special constructor for use constructing initial 'auto' value.
    GridTrackSizes(css::keywords::Auto) noexcept
        : css::ListOrDefault<GridTrackSizeList, GridTrackSizeDefaulter> {DefaultValue}
    {
    }
  };
}

DEFINE_RANGE_LIKE_CONFORMANCE_FOR_LIST_OR_DEFAULT_DERIVED_TYPE(krys::boo::style::GridTrackSizes)
