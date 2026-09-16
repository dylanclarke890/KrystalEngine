#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  struct SourceLocation
  {
    size_t Line {1uz};
    size_t Column {1uz};
  };
}