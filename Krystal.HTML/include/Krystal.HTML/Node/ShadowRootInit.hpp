#pragma once

#include "Krystal.HTML/Node/ShadowRootMode.hpp"
#include "Krystal.HTML/Node/SlotAssignmentMode.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;

  struct ShadowRootInit
  {
    ShadowRootMode Mode : BitCount<ShadowRootMode>() {ShadowRootMode::Open};
    bool DelegatesFocus : 1 {false};
    SlotAssignmentMode SlotAssignment : BitCount<SlotAssignmentMode>() {SlotAssignmentMode::Named};
    bool Clonable : 1 {false};
    bool Serializable : 1 {false};
    RawPtr<CustomElementRegistry> CustomElementRegistry {nullptr};
  };
}