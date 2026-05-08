#include "Krystal.HTML/Events/EventPath.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Events/EventDispatcher.hpp"
#include "Krystal.HTML/Events/EventNames.hpp"
#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

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
