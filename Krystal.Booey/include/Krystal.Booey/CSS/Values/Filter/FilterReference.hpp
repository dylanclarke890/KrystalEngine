#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  // https://drafts.fxtf.org/filter-effects/#typedef-filter-url
  struct FilterReference
  {
    CSSURL url;

    bool operator==(const FilterReference &) const = default;
  };

  DEFINE_TYPE_WRAPPER_GET(FilterReference, url);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::FilterReference, 1)
