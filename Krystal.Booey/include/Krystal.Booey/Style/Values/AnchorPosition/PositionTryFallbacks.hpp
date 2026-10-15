#pragma once

#include "Krystal.Booey/Style/Values/AnchorPosition/PositionTryFallback.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <position-try-fallback-list> = <position-try-fallback>#
  using PositionTryFallbackList = css::CommaSeparatedFixedList<PositionTryFallback>;

  // <'position-try-fallbacks'> = none | <position-try-callback-list>
  // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-try-fallbacks
  struct PositionTryFallbacks : css::ListOrNone<PositionTryFallbackList>
  {
    using css::ListOrNone<PositionTryFallbackList>::ListOrNone;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::PositionTryFallbacks)
