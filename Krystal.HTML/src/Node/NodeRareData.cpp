#include "Krystal.HTML/Node/NodeRareData.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/NodeList/ChildNodeList.hpp"

namespace Krys::HTML
{
  Ref<NodeList> NodeRareData::ChildNodes(Node &node) noexcept
  {
    if (!_childNodeList)
    {
      auto children = CreateRef<ChildNodeList>(node);
      _childNodeList = CreateWeakPtr<NodeList>(children.get());
      return children;
    }

    return ShareRef(*_childNodeList.get());
  }

  void NodeRareData::InvalidateChildNodes() noexcept
  {
    if (_childNodeList)
    {
      _childNodeList->Invalidate();
    }
  }

  List<Ref<RegisteredObserver>> &NodeRareData::RegisteredObserverList() noexcept
  {
    if (!_registeredObserverList.has_value())
    {
      _registeredObserverList = {};
    }

    return *_registeredObserverList;
  }

  List<Ref<TransientRegisteredObserver>> &NodeRareData::TransientRegisteredObservers() noexcept
  {
    if (!_transientRegisteredObservers.has_value())
    {
      _transientRegisteredObservers = {};
    }

    return *_transientRegisteredObservers;
  }
}
