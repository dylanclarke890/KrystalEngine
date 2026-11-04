#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief How much of a run of the selection (or another highlight) an object holds.
  enum class RenderObjectHighlightState : uint8
  {
    None,   // The object is not selected.
    Start,  // The object either contains the start of a selection run or is the start of a run
    Inside, // The object is fully encompassed by a selection run
    End,    // The object either contains the end of a selection run or is the end of a run
    Both    // The object contains an entire run or is the sole selected object in that run
  };
}
