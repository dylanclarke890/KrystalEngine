#include "Krystal.Booey/DOM/Internals/RareData/NodeRareData.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Internals/MutationObserver/TransientRegisteredObserver.hpp"
#include "Krystal.Booey/DOM/Internals/NodeList/LiveNodeList.hpp"
#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom
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
