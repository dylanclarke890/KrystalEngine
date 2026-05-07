#include "Krystal.HTML/Node/RareData/NodeRareData.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  Ref<NodeList> NodeRareData::ChildNodes(Node &node) noexcept
  {
    if (auto childNodes = _childNodeList.lock())
    {
      return ShareRef(*childNodes.get());
    }

    auto childNodes = CreateRef<LiveNodeList>(CreateWeakRef<Node>(node),
                                              [&](const Node &n) { return n.ParentNode() == &node; });
    _childNodeList = CreateWeakPtr<NodeList>(childNodes.get());

    return childNodes;
  }

  List<Ref<RegisteredObserver>> &NodeRareData::RegisteredObserverList() noexcept
  {
    if (!_registeredObserverList.has_value())
    {
      _registeredObserverList = List<Ref<RegisteredObserver>> {};
    }

    return _registeredObserverList.value();
  }

  List<Ref<TransientRegisteredObserver>> &NodeRareData::TransientRegisteredObservers() noexcept
  {
    if (!_transientRegisteredObservers.has_value())
    {
      _transientRegisteredObservers = List<Ref<TransientRegisteredObserver>> {};
    }

    return _transientRegisteredObservers.value();
  }
}
