#pragma once

#include "Krystal.Booey/Animation/ScrollAxis.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <single-progress-timeline-axis> = block | inline | x | y
  using ProgressTimelineAxis = ScrollAxis;

  // <progress-timeline-axis-list> = <single-progress-timeline-axis>#
  using ProgressTimelineAxisList = css::CommaSeparatedFixedList<ProgressTimelineAxis>;

  // Default value for <'scroll-timeline-axis'> and <'view-timeline-axis'> is 'block'.
  struct ProgressTimelineAxisDefaulter
  {
    KRYS_NODISCARD const ProgressTimelineAxis &operator()() const noexcept
    {
      constexpr static auto value = ProgressTimelineAxis::Block;
      return value;
    }

    KRYS_NODISCARD constexpr bool operator==(const ProgressTimelineAxisDefaulter &) const noexcept = default;
  };

  // <'scroll-timeline-axis'> = <progress-timeline-axis-list>
  // https://drafts.csswg.org/scroll-animations-1/#propdef-scroll-timeline-axis
  // <'view-timeline-axis'> = <progress-timeline-axis-list>
  // https://drafts.csswg.org/scroll-animations-1/#propdef-view-timeline-axis
  struct ProgressTimelineAxes : css::ListOrDefault<ProgressTimelineAxisList, ProgressTimelineAxisDefaulter>
  {
    using css::ListOrDefault<ProgressTimelineAxisList, ProgressTimelineAxisDefaulter>::ListOrDefault;

    // Special constructor for use constructing initial 'block' value.
    ProgressTimelineAxes(css::keywords::Block) noexcept
        : css::ListOrDefault<ProgressTimelineAxisList, ProgressTimelineAxisDefaulter> {DefaultValue}
    {
    }
  };
}

DEFINE_RANGE_LIKE_CONFORMANCE_FOR_LIST_OR_DEFAULT_DERIVED_TYPE(krys::boo::style::ProgressTimelineAxes)
