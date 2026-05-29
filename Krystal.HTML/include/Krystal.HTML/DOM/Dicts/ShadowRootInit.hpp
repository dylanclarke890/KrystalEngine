#pragma once

#include "Krystal.HTML/DOM/Enums/ShadowRootMode.hpp"
#include "Krystal.HTML/DOM/Enums/SlotAssignmentMode.hpp"

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

  /// @see https://dom.spec.whatwg.org/#dictdef-shadowrootinit
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