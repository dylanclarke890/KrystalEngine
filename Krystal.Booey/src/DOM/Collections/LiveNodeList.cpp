#include "Krystal.Booey/DOM/Collections/LiveNodeList.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Core/Move.hpp"

namespace krys::boo
{
  LiveNodeList::LiveNodeList(Node &root, LiveNodeListFilterFunc filter,
                             dom::LiveCollectionInvalidation invalidation,
                             dom::LiveCollectionTraversal traversal) noexcept
      : dom::LiveCollection(root, invalidation, traversal), _filter(krys::move(filter))
  {
  }

#pragma region NodeList - https://dom.spec.whatwg.org/#nodelist

  RefPtr<Node> LiveNodeList::Item(size_t index) noexcept
  {
    return ShareRefPtr(_indexCache.At(*this, index));
  }

  RefPtr<const Node> LiveNodeList::Item(size_t index) const noexcept
  {
    return const_cast<LiveNodeList *>(this)->Item(index);
  }

  RefPtr<Node> LiveNodeList::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<const Node> LiveNodeList::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  size_t LiveNodeList::Length() const noexcept
  {
    return _indexCache.Count(*this);
  }

#pragma endregion

  void LiveNodeList::InvalidateCache() noexcept
  {
    _indexCache.Invalidate();
    UnregisterCache();
  }

  void LiveNodeList::WillValidateIndexCache() const noexcept
  {
    RegisterCache();
  }

  Node *LiveNodeList::CollectionBegin() const noexcept
  {
    return FirstMatchFrom(FirstNode());
  }

  Node *LiveNodeList::CollectionNext(Node &node) const noexcept
  {
    return FirstMatchFrom(NextNode(node));
  }

  Node *LiveNodeList::FirstMatchFrom(Node *node) const noexcept
  {
    for (; node; node = NextNode(*node))
    {
      if (_filter(*node))
      {
        return node;
      }
    }

    return nullptr;
  }
}
