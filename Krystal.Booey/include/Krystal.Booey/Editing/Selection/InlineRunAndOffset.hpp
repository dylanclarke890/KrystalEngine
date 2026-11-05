#pragma once

#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBox.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief The leaf box on a line a position renders in, and its caret offset there.
  struct InlineBoxAndOffset
  {
    inline_iterator::LeafBoxIterator Box {};
    uint32 Offset {0};
  };
}
