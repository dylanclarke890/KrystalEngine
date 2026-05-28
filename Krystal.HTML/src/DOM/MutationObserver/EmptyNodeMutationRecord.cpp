#include "Krystal.HTML/DOM/MutationObserver/EmptyNodeMutationRecord.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Collections/StaticNodeList.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/Node.hpp"

namespace Krys::HTML
{
  EmptyNodeMutationRecord::EmptyNodeMutationRecord(Ref<Node> &&target, DOMStringAtom oldValue) noexcept
      : _target(Krys::Move(target)), _addedNodes(nullptr), _removedNodes(nullptr), _oldValue(oldValue)
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
