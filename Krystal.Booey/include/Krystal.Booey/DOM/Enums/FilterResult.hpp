#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief What a filter returns for a node.
  /// @see https://dom.spec.whatwg.org/#interface-nodefilter
  enum class FilterResult : uint8
  {
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-filter_accept
    FILTER_ACCEPT = 1,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-filter_reject
    FILTER_REJECT = 2,
    /// @see https://dom.spec.whatwg.org/#dom-nodefilter-filter_skip
    FILTER_SKIP = 3,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::FilterResult, 3u);