#include "Krystal.Booey/DOM/Internals/MutationRecord/ChildListMutationRecord.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"

namespace krys::boo::dom
{
  ChildListMutationRecord::ChildListMutationRecord(Ref<Node> &&target, Ref<NodeList> &&added,
                                                   Ref<NodeList> &&removed, RefPtr<Node> &&previousSibling,
                                                   RefPtr<Node> &&nextSibling) noexcept
      : _target(krys::move(target)), _addedNodes(krys::move(added)), _removedNodes(krys::move(removed)),
        _previousSibling(krys::move(previousSibling)), _nextSibling(krys::move(nextSibling))
  {
  }
}
