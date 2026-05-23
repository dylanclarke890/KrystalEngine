#pragma once

#include "Krystal.Lib/Mixins/RefCounted.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class EventTarget;

  class EventPathItem
  {
  private:
    Ref<EventTarget> _invocationTarget;
    RefPtr<EventTarget> _shadowAdjustedTarget {nullptr};
    RefPtr<EventTarget> _relatedTarget {nullptr};
    List<Ref<EventTarget>> _touchTargetList;
    bool _invocationTargetInShadowTree : 1 {false};
    bool _rootOfClosedTree : 1 {false};
    bool _slotInClosedTree : 1 {false};

  public:
    EventPathItem(EventTarget &invocationTarget, bool invocationTargetInShadowTree,
                  RawPtr<EventTarget> shadowAdjustedTarget, RawPtr<EventTarget> relatedTarget,
                  List<Ref<EventTarget>> &touchTargetList, bool rootOfClosedTree,
                  bool slotInClosedTree) noexcept;

    KRYS_NODISCARD RawPtr<EventTarget> InvocationTarget() const noexcept
    {
      return _invocationTarget.get();
    }

    KRYS_NODISCARD RawPtr<EventTarget> ShadowAdjustedTarget() const noexcept
    {
      return _shadowAdjustedTarget.get();
    }

    KRYS_NODISCARD RawPtr<EventTarget> RelatedTarget() const noexcept
    {
      return _relatedTarget.get();
    }

    KRYS_NODISCARD const List<Ref<EventTarget>> &TouchTargetList() const noexcept
    {
      return _touchTargetList;
    }

    KRYS_NODISCARD bool InvocationTargetInShadowTree() const noexcept
    {
      return _invocationTargetInShadowTree;
    }

    KRYS_NODISCARD bool RootOfClosedTree() const noexcept
    {
      return _rootOfClosedTree;
    }

    KRYS_NODISCARD bool SlotInClosedTree() const noexcept
    {
      return _slotInClosedTree;
    }
  };
}