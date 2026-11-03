#include "Krystal.Booey/DOM/Internals/NodeRareData.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Collections/LiveNodeList.hpp"
#include "Krystal.Booey/DOM/MutationObservers/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationObservers/TransientRegisteredObserver.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom
{
  NodeRareData::~NodeRareData() noexcept = default;

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

  Ref<HTMLCollection> NodeRareData::Children(ContainerNode &parent) noexcept
  {
    if (auto children = _children.lock())
    {
      return ShareRef(*children.get());
    }

    auto children =
      CreateRef<LiveHTMLCollection>(parent, [&](const Element &n) { return n.ParentNode() == &parent; });
    _children = CreateWeakPtr(children.get());

    return children;
  }

  WeakPtr<NodeList> &NodeRareData::LabelsNodeList() noexcept
  {
    return _labelsNodeList;
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
