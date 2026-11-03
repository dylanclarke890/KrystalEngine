#include "Krystal.Booey/DOM/Events/EventPathItem.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Events/EventTarget.hpp"

namespace krys::boo::dom
{
  EventPathItem::EventPathItem(EventTarget &invocationTarget, bool invocationTargetInShadowTree,
                               EventTarget *shadowAdjustedTarget, EventTarget *relatedTarget,
                               List<Ref<EventTarget>> &touchTargetList, bool rootOfClosedTree,
                               bool slotInClosedTree) noexcept
      : _invocationTarget(ShareRef(invocationTarget)),
        _shadowAdjustedTarget(ShareRefPtr(shadowAdjustedTarget)), _relatedTarget(ShareRefPtr(relatedTarget)),
        _touchTargetList(touchTargetList), _invocationTargetInShadowTree(invocationTargetInShadowTree),
        _rootOfClosedTree(rootOfClosedTree), _slotInClosedTree(slotInClosedTree)
  {
  }
}