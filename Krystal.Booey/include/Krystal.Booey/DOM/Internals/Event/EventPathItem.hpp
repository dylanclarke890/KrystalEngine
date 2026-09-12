#pragma once

#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class EventTarget;

  /// @see https://dom.spec.whatwg.org/#event-path
  class EventPathItem
  {
  private:
    Ref<EventTarget> _invocationTarget;
    RefPtr<EventTarget> _shadowAdjustedTarget {nullptr};
    RefPtr<EventTarget> _relatedTarget {nullptr};
    List<Ref<EventTarget>> _touchTargetList;
    bool _invocationTargetInShadowTree {false};
    bool _rootOfClosedTree {false};
    bool _slotInClosedTree {false};

  public:
    EventPathItem(EventTarget &invocationTarget, bool invocationTargetInShadowTree,
                  EventTarget *shadowAdjustedTarget, EventTarget *relatedTarget,
                  List<Ref<EventTarget>> &touchTargetList, bool rootOfClosedTree,
                  bool slotInClosedTree) noexcept;

    /// @see https://dom.spec.whatwg.org/#event-path-invocation-target
    KRYS_NODISCARD EventTarget *InvocationTarget() const noexcept
    {
      return _invocationTarget.get();
    }

    /// @see https://dom.spec.whatwg.org/#event-path-shadow-adjusted-target
    KRYS_NODISCARD EventTarget *ShadowAdjustedTarget() const noexcept
    {
      return _shadowAdjustedTarget.get();
    }

    /// @see https://dom.spec.whatwg.org/#event-path-related-target
    KRYS_NODISCARD EventTarget *RelatedTarget() const noexcept
    {
      return _relatedTarget.get();
    }

    /// @see https://dom.spec.whatwg.org/#event-path-touch-target-list
    KRYS_NODISCARD const List<Ref<EventTarget>> &TouchTargetList() const noexcept
    {
      return _touchTargetList;
    }

    /// @see https://dom.spec.whatwg.org/#event-path-invocation-target-in-shadow-tree
    KRYS_NODISCARD bool InvocationTargetInShadowTree() const noexcept
    {
      return _invocationTargetInShadowTree;
    }

    /// @see https://dom.spec.whatwg.org/#event-path-root-of-closed-tree
    KRYS_NODISCARD bool RootOfClosedTree() const noexcept
    {
      return _rootOfClosedTree;
    }

    /// @see https://dom.spec.whatwg.org/#event-path-slot-in-closed-tree
    KRYS_NODISCARD bool SlotInClosedTree() const noexcept
    {
      return _slotInClosedTree;
    }
  };
}