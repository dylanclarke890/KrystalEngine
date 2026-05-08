#pragma once

#include "Krystal.HTML/Node/Enums/ShadowRootMode.hpp"
#include "Krystal.HTML/Node/Enums/SlotAssignmentMode.hpp"
#include "Krystal.HTML/Node/ShadowRootInit.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;
  class Element;
  class EventTarget;
  class Node;
  class ShadowRoot;

  /// @see https://dom.spec.whatwg.org/#interface-shadowroot
  class ShadowRootAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-root
    KRYS_NODISCARD static const Node &ShadowIncludingRoot(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-root
    KRYS_NODISCARD static Node &ShadowIncludingRoot(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-descendant
    KRYS_NODISCARD static bool IsShadowIncludingDescendant(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-inclusive-descendant
    KRYS_NODISCARD static bool IsShadowIncludingInclusiveDescendant(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-ancestor
    KRYS_NODISCARD static bool IsShadowIncludingAncestor(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-inclusive-ancestor
    KRYS_NODISCARD static bool IsShadowIncludingInclusiveAncestor(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-closed-shadow-hidden
    KRYS_NODISCARD static bool IsClosedShadowHidden(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#retarget
    KRYS_NODISCARD static RawPtr<EventTarget> Retarget(RawPtr<EventTarget> a, EventTarget &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-attach-a-shadow-root
    KRYS_NODISCARD static ExceptionOr<Ref<ShadowRoot>>
      AttachShadowRoot(Element &element, ShadowRootMode mode, Clonable clonable, Serializable serializable,
                       DelegatesFocus delegatesFocus, SlotAssignmentMode slotAssignment,
                       RawPtr<CustomElementRegistry> registry) noexcept;

    /// @see https://dom.spec.whatwg.org/#element-shadow-host
    KRYS_NODISCARD static bool IsShadowHost(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#element-shadow-host
    KRYS_NODISCARD static bool IsShadowHost(const Element &node) noexcept;

    KRYS_NODISCARD static RefPtr<ShadowRoot> GetShadowRoot(const Node &node) noexcept;

    KRYS_NODISCARD static bool HasSameShadowIncludingRoot(const Node &a, const Node &b) noexcept;
  };
}