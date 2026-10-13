#pragma once

#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"

namespace krys::boo::style
{
  // This state has lifetime of a single style resolution cycle, including any layout interleaving.
  struct TreeResolutionState : public CanMakeCheckedPtr<TreeResolutionState>
  {
    KRYS_STRUCT_OVERRIDE_DELETE_FOR_CHECKED_PTR(TreeResolutionState);

    AnchorPositionedStates anchorPositionedStates;
  };
}
