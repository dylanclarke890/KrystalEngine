#pragma once

#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"

namespace krys::boo::style
{
  /// @brief This state has lifetime of a single style resolution cycle, including any layout interleaving.
  struct TreeResolutionState
  {
    style::AnchorPositionedStates AnchorPositionedStates;
  };
}
