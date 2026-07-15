#pragma once

#include "Krystal.Lib/Types/Numeric.hpp"

namespace Krys::HTML
{
  struct SourceLocation
  {
    size_t Line {1uz};
    size_t Column {1uz};
  };
}