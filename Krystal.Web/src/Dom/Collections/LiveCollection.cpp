#include "Krystal.Web/Dom/Collections/LiveCollection.hpp"
#include "Krystal.Web/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Node.hpp"

namespace krys::web::dom
{
  bool LiveCollectionDependsOnAttribute(LiveCollectionInvalidation invalidation,
                                        DOMStringView localName) noexcept
  {
    switch (invalidation)
    {
      case LiveCollectionInvalidation::TreeOnly: return false;
      case LiveCollectionInvalidation::Class:    return localName == u8"class";
      case LiveCollectionInvalidation::IdOrName: return localName == u8"id" || localName == u8"name";
      case LiveCollectionInvalidation::Name:     return localName == u8"name";
      case LiveCollectionInvalidation::Href:     return localName == u8"href";
      case LiveCollectionInvalidation::Labels:
        return localName == u8"for" || localName == u8"id" || localName == u8"type";
      case LiveCollectionInvalidation::FormControls:
        return localName == u8"id" || localName == u8"name" || localName == u8"form" || localName == u8"type";
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

  void LiveCollection::InvalidateCacheForAttribute(DOMStringView localName) noexcept
  {
    if (LiveCollectionDependsOnAttribute(_invalidation, localName))
    {
      InvalidateCache();
    }
  }

  Node *LiveCollection::FirstNode() const noexcept
  {
    return _traversal == LiveCollectionTraversal::OwnerTree ? TreeQueries::Root(*_root).FirstChild()
                                                            : _root->FirstChild();
  }

  Node *LiveCollection::NextNode(const Node &node) const noexcept
  {
    switch (_traversal)
    {
      case LiveCollectionTraversal::Children:  return node.NextSibling();
      case LiveCollectionTraversal::OwnerTree: return TreeQueries::Next(node);
      default:                                 return TreeQueries::Next(node, _root.get());
    }
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
      
      auto &document = _root->NodeDocument();
      document.LiveCollectionCacheAdded(_invalidation);
      
      if (_traversal == LiveCollectionTraversal::OwnerTree)
      {
        document.AddOwnerTreeLiveCollectionCache(const_cast<LiveCollection &>(*this));
      }
    }
  }

  void LiveCollection::UnregisterCache() const noexcept
  {
    if (_isRegistered)
    {
      _isRegistered = false;
      _root->RareData()->RemoveLiveCollectionCache(const_cast<LiveCollection &>(*this));
      
      auto &document = _root->NodeDocument();
      document.LiveCollectionCacheRemoved(_invalidation);
      
      if (_traversal == LiveCollectionTraversal::OwnerTree)
      {
        document.RemoveOwnerTreeLiveCollectionCache(const_cast<LiveCollection &>(*this));
      }
    }
  }
}
