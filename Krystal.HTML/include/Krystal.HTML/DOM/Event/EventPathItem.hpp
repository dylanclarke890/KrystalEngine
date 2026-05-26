#pragma once

#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
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
                  RawPtr<EventTarget> shadowAdjustedTarget, RawPtr<EventTarget> relatedTarget,
                  List<Ref<EventTarget>> &touchTargetList, bool rootOfClosedTree,
                  bool slotInClosedTree) noexcept;

    /// @see https://dom.spec.whatwg.org/#event-path-invocation-target
    KRYS_NODISCARD RawPtr<EventTarget> InvocationTarget() const noexcept
    {
      return _invocationTarget.get();
    }

    /// @see https://dom.spec.whatwg.org/#event-path-shadow-adjusted-target
    KRYS_NODISCARD RawPtr<EventTarget> ShadowAdjustedTarget() const noexcept
    {
      return _shadowAdjustedTarget.get();
    }

    /// @see https://dom.spec.whatwg.org/#event-path-related-target
    KRYS_NODISCARD RawPtr<EventTarget> RelatedTarget() const noexcept
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