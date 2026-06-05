#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#concept-traversal-filter
  enum class FilterResult : uint8
  {
    FILTER_ACCEPT = 1,
    FILTER_REJECT = 2,
    FILTER_SKIP = 3,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::FilterResult, 3u);