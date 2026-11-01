#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief What a filter returns for a node.
  /// @see https://dom.spec.whatwg.org/#interface-nodefilter
  enum class FilterResult : uint8
  {
    FILTER_ACCEPT = 1,
    FILTER_REJECT = 2,
    FILTER_SKIP = 3,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::FilterResult, 3u);