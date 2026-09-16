#pragma once

#include "Krystal.Booey/DOM/Enums/ShadowRootMode.hpp"
#include "Krystal.Booey/DOM/Enums/SlotAssignmentMode.hpp"

namespace krys::boo::html
{
  class CustomElementRegistry;
}

namespace krys::boo::dom
{
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
    html::CustomElementRegistry *CustomElementRegistry {nullptr};
  };
}