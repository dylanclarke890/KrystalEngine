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
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"

namespace krys::boo::dom
{
  NodeRareData::~NodeRareData() noexcept = default;

  Ref<NodeList> NodeRareData::ChildNodes(Node &node) noexcept
  {
    if (auto childNodes = _childNodeList.lock())
    {
      return ShareRef(*childNodes);
    }

    auto childNodes = CreateRef<LiveNodeList>(
      node, [](const Node &) { return true; }, LiveCollectionInvalidation::TreeOnly,
      LiveCollectionTraversal::Children);
    _childNodeList = CreateWeakPtr<NodeList>(childNodes.get());

    return childNodes;
  }

  Ref<HTMLCollection> NodeRareData::Children(ContainerNode &parent) noexcept
  {
    if (auto children = _children.lock())
    {
      return ShareRef(*children.get());
    }

    auto children = CreateRef<LiveHTMLCollection>(
      parent, [](const Element &) { return true; }, LiveCollectionInvalidation::TreeOnly,
      LiveCollectionTraversal::Children);
    _children = CreateWeakPtr(children.get());

    return children;
  }

  WeakPtr<NodeList> &NodeRareData::LabelsNodeList() noexcept
  {
    return _labelsNodeList;
  }

  void NodeRareData::AddLiveCollectionCache(LiveCollection &collection) noexcept
  {
    _liveCollectionCaches.push_back(&collection);
  }

  void NodeRareData::RemoveLiveCollectionCache(LiveCollection &collection) noexcept
  {
    auto it = std::ranges::find(_liveCollectionCaches, &collection);
    krys_debug_assert(it != _liveCollectionCaches.end());
    *it = _liveCollectionCaches.back();
    _liveCollectionCaches.pop_back();
  }

  void NodeRareData::InvalidateLiveCollectionCaches() noexcept
  {
    // Last first: each removes itself, which moves the last into its place.
    for (auto i = _liveCollectionCaches.size(); i-- > 0;)
    {
      _liveCollectionCaches[i]->InvalidateCache();
    }
  }

  void NodeRareData::InvalidateLiveCollectionCachesForAttribute(DOMStringAtom localName) noexcept
  {
    // Last first: each that forgets its items removes itself, which moves the last into its place.
    for (auto i = _liveCollectionCaches.size(); i-- > 0;)
    {
      _liveCollectionCaches[i]->InvalidateCacheForAttribute(localName);
    }
  }

  void NodeRareData::InvalidateChildNodeListCache() noexcept
  {
    if (auto *childNodes = _childNodeList.get())
    {
      Downcast<LiveNodeList>(*childNodes).InvalidateCache();
    }
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
