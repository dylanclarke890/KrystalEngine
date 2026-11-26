#include "Krystal.Web/Dom/Events/EventPathItem.hpp"
#include "Krystal.Web/Dom/Events/EventTarget.hpp"

namespace krys::web::dom
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
