#include "Krystal.HTML/DOM/MutationObserver/ChildListMutationRecord.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"

namespace Krys::HTML
{
  ChildListMutationRecord::ChildListMutationRecord(Ref<Node> &&target, Ref<NodeList> &&added,
                                                   Ref<NodeList> &&removed, RefPtr<Node> &&previousSibling,
                                                   RefPtr<Node> &&nextSibling) noexcept
      : _target(Krys::Move(target)), _addedNodes(Krys::Move(added)), _removedNodes(Krys::Move(removed)),
        _previousSibling(Krys::Move(previousSibling)), _nextSibling(Krys::Move(nextSibling))
  {
  }
}
