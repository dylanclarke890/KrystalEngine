#include "Krystal.HTML/DOM/Internals/RareData/NodeRareData.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/Internals/MutationObserver/TransientRegisteredObserver.hpp"
#include "Krystal.HTML/DOM/Internals/NodeList/LiveNodeList.hpp"
#include "Krystal.HTML/DOM/MutationObserver.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  Ref<NodeList> NodeRareData::ChildNodes(Node &node) noexcept
  {
    if (auto childNodes = _childNodeList.lock())
    {
      return ShareRef(*childNodes);
    }

    auto childNodes = CreateRef<LiveNodeList>(node, [&](const Node &n) { return n.ParentNode() == &node; });
    _childNodeList = CreateWeakPtr<NodeList>(childNodes.get());

    return childNodes;
  }

  List<Ref<RegisteredObserver>> &NodeRareData::RegisteredObservers() noexcept
  {
    return _registeredObservers;
  }

  List<Ref<TransientRegisteredObserver>> &NodeRareData::TransientRegisteredObservers() noexcept
  {
    return _transientRegisteredObservers;
  }
}
