#pragma once

#include "Krystal.HTML/Node/ShadowRootMode.hpp"
#include "Krystal.HTML/Node/SlotAssignmentMode.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/StronglyTypedValue.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;

  struct DelegatesFocus : public StronglyTypedBool<DelegatesFocus>
  {
    using Base::Base;
  };

  struct Clonable : public StronglyTypedBool<Clonable>
  {
    using Base::Base;
  };

  struct Serializable : public StronglyTypedBool<Serializable>
  {
    using Base::Base;
  };

  struct ShadowRootInit
  {
    ShadowRootMode Mode {ShadowRootMode::Open};
    DelegatesFocus DelegatesFocus {false};
    SlotAssignmentMode SlotAssignment {SlotAssignmentMode::Named};
    Clonable Clonable {false};
    Serializable Serializable {false};
    RawPtr<CustomElementRegistry> CustomElementRegistry {nullptr};
  };
}