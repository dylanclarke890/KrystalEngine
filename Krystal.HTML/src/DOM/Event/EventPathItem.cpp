#include "Krystal.HTML/DOM/Event/EventPathItem.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/DOM/EventTarget.hpp"

namespace Krys::HTML
{
  EventPathItem::EventPathItem(EventTarget &invocationTarget, bool invocationTargetInShadowTree,
                               RawPtr<EventTarget> shadowAdjustedTarget, RawPtr<EventTarget> relatedTarget,
                               List<Ref<EventTarget>> &touchTargetList, bool rootOfClosedTree,
                               bool slotInClosedTree) noexcept
      : _invocationTarget(ShareRef(invocationTarget)),
        _shadowAdjustedTarget(ShareRefPtr(shadowAdjustedTarget)), _relatedTarget(ShareRefPtr(relatedTarget)),
        _touchTargetList(touchTargetList), _invocationTargetInShadowTree(invocationTargetInShadowTree),
        _rootOfClosedTree(rootOfClosedTree), _slotInClosedTree(slotInClosedTree)
  {
  }
}