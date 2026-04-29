#include "Krystal.HTML/MutationObserver/detail/EmptyNodeMutationRecord.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/StaticNodeList.hpp"

namespace Krys::HTML::detail
{
  EmptyNodeMutationRecord::EmptyNodeMutationRecord(Node &target, const DOMString &oldValue) noexcept
      : _target(ShareRef<Node>(target)), _addedNodes(nullptr), _removedNodes(nullptr), _oldValue(oldValue)
  {
  }

  RawPtr<NodeList> EmptyNodeMutationRecord::AddedNodes() const noexcept
  {
    if (!_addedNodes)
    {
      _addedNodes = CreateRefPtr<StaticNodeList>();
    }

    return _addedNodes.get();
  }

  RawPtr<NodeList> EmptyNodeMutationRecord::RemovedNodes() const noexcept
  {
    if (!_removedNodes)
    {
      _removedNodes = CreateRefPtr<StaticNodeList>();
    }

    return _removedNodes.get();
  }
}
