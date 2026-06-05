#include "Krystal.HTML/DOM/Internals/MutationRecord/ChildListMutationRecord.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"

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
