#include "Krystal.Booey/DOM/Internals/MutationRecord/EmptyNodeMutationRecord.hpp"
#include "Krystal.Booey/DOM/Internals/NodeList/StaticNodeList.hpp"

namespace krys::boo::dom
{
  EmptyNodeMutationRecord::EmptyNodeMutationRecord(Ref<Node> &&target, DOMStringAtom oldValue) noexcept
      : _target(krys::move(target)), _addedNodes(nullptr), _removedNodes(nullptr), _oldValue(oldValue)
  {
  }

  RefPtr<NodeList> EmptyNodeMutationRecord::AddedNodes() const noexcept
  {
    if (_addedNodes == nullptr)
    {
      _addedNodes = CreateRefPtr<StaticNodeList>();
    }

    return _addedNodes;
  }

  RefPtr<NodeList> EmptyNodeMutationRecord::RemovedNodes() const noexcept
  {
    if (_removedNodes == nullptr)
    {
      _removedNodes = CreateRefPtr<StaticNodeList>();
    }

    return _removedNodes;
  }
}
