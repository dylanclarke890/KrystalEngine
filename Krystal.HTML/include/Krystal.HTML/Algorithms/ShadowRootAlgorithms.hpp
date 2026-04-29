#pragma once

#include "Krystal.HTML/Node/ShadowRootInit.hpp"
#include "Krystal.HTML/Node/ShadowRootMode.hpp"
#include "Krystal.HTML/Node/SlotAssignmentMode.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;
  class Element;
  class ShadowRoot;

  class ShadowRootAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-attach-a-shadow-root
    KRYS_NODISCARD static ExceptionOr<Ref<ShadowRoot>>
      AttachShadowRoot(Element &element, ShadowRootMode mode, Clonable clonable, Serializable serializable,
                       DelegatesFocus delegatesFocus, SlotAssignmentMode slotAssignment,
                       RawPtr<CustomElementRegistry> registry) noexcept;
  };
}