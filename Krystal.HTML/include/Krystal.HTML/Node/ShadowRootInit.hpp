#pragma once

#include "Krystal.HTML/Node/ShadowRootMode.hpp"
#include "Krystal.HTML/Node/SlotAssignmentMode.hpp"

namespace Krys::HTML
{
  struct ShadowRootInit
  {
    ShadowRootMode Mode : BitCount<ShadowRootMode>() {ShadowRootMode::Open};
    bool DelegatesFocus : 1 {false};
    SlotAssignmentMode SlotAssignment : BitCount<SlotAssignmentMode>() {SlotAssignmentMode::Manual};
    bool Clonable : 1 {false};
    bool Serializable : 1 {false};
  };
}