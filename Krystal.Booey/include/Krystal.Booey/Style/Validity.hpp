#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::style
{
  /// @brief How much of a node's style needs resolving again, in increasing order.
  enum class Validity : uint8
  {
    Valid,
    AnimationInvalid,
    InlineStyleInvalid,
    ElementInvalid,
    SubtreeInvalid,
  };

  /// @brief What else an invalidation asks of the next style resolution.
  enum class InvalidationMode : uint8
  {
    Normal,
    RecompositeLayer,
    RebuildRenderer,
    InsertedIntoAncestor
  };

  enum class InvalidationScope : uint8
  {
    All,
    SelfChildrenAndSiblings,
    Descendants
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::style::Validity, 5u);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::style::InvalidationMode, 4u);
KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::style::InvalidationScope, 3u);
