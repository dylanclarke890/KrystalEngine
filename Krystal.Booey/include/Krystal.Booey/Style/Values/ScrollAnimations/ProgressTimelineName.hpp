#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <single-progress-timeline-name> = none | <dashed-ident>
  // FIXME: This should actually model the union of css::keywords::None and css::CustomIdentifier to match the
  // spec - https://bugs.webkit.org/show_bug.cgi?id=295467.
  struct ProgressTimelineName
  {
    css::CustomIdentifier Value;

    KRYS_NODISCARD bool operator==(const ProgressTimelineName &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(ProgressTimelineName, Value);

  // <progress-timeline-name-list> = <single-progress-timeline-name>#
  using ProgressTimelineNameList = css::CommaSeparatedFixedList<ProgressTimelineName>;

  // <'scroll-timeline-name'> = <progress-timeline-name-list>
  // https://drafts.csswg.org/scroll-animations-1/#propdef-scroll-timeline-name
  // <'view-timeline-name'> = <progress-timeline-name-list>
  // https://drafts.csswg.org/scroll-animations-1/#propdef-view-timeline-name
  struct ProgressTimelineNames : css::ListOrNone<ProgressTimelineNameList>
  {
    using css::ListOrNone<ProgressTimelineNameList>::ListOrNone;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ProgressTimelineName>
  {
    KRYS_NODISCARD ProgressTimelineName operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::ProgressTimelineName, 1)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ProgressTimelineNames)
