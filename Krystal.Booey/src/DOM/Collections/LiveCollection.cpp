#include "Krystal.Booey/DOM/Collections/LiveCollection.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"

namespace krys::boo::dom
{
  bool LiveCollectionDependsOnAttribute(LiveCollectionInvalidation invalidation,
                                        DOMStringAtom localName) noexcept
  {
    switch (invalidation)
    {
      case LiveCollectionInvalidation::TreeOnly: return false;
      case LiveCollectionInvalidation::Class:    return localName == u8"class";
      case LiveCollectionInvalidation::IdOrName: return localName == u8"id" || localName == u8"name";
      case LiveCollectionInvalidation::Name:     return localName == u8"name";
      case LiveCollectionInvalidation::Href:     return localName == u8"href";
      case LiveCollectionInvalidation::Count:    break;
    }
    return false;
  }

  LiveCollection::LiveCollection(Node &root, LiveCollectionInvalidation invalidation,
                                 LiveCollectionTraversal traversal) noexcept
      : _root(ShareRef(root)), _invalidation(invalidation), _traversal(traversal)
  {
  }

  LiveCollection::~LiveCollection() noexcept
  {
    UnregisterCache();
  }

  void LiveCollection::InvalidateCacheForAttribute(DOMStringAtom localName) noexcept
  {
    if (LiveCollectionDependsOnAttribute(_invalidation, localName))
    {
      InvalidateCache();
    }
  }

  Node *LiveCollection::FirstNode() const noexcept
  {
    return _root->FirstChild();
  }

  Node *LiveCollection::NextNode(const Node &node) const noexcept
  {
    return _traversal == LiveCollectionTraversal::Children ? node.NextSibling()
                                                           : TreeQueries::Next(node, _root.get());
  }

  Node &LiveCollection::Root() const noexcept
  {
    return *_root;
  }

  void LiveCollection::RegisterCache() const noexcept
  {
    if (!_isRegistered)
    {
      _isRegistered = true;
      _root->EnsureNodeRareData().AddLiveCollectionCache(const_cast<LiveCollection &>(*this));
      _root->NodeDocument().LiveCollectionCacheAdded(_invalidation);
    }
  }

  void LiveCollection::UnregisterCache() const noexcept
  {
    if (_isRegistered)
    {
      _isRegistered = false;
      _root->RareData()->RemoveLiveCollectionCache(const_cast<LiveCollection &>(*this));
      _root->NodeDocument().LiveCollectionCacheRemoved(_invalidation);
    }
  }
}
