#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys
{
  /// @brief A position in source text, from line 1 and column 1.
  struct SourceLocation
  {
    size_t Line {1uz};
    size_t Column {1uz};
  };
}
