#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSURL.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // https://drafts.fxtf.org/filter-effects/#typedef-filter-url
  struct FilterReference
  {
    CSSURL url;

    bool operator==(const FilterReference &) const = default;
  };

  DEFINE_TYPE_WRAPPER_GET(FilterReference, url);
}

DEFINE_TUPLE_LIKE_CONFORMANCE(Krys::HTML::FilterReference, 1)
