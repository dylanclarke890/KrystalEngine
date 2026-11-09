#include "Krystal.Booey/DOM/Collections/LiveNodeList.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::boo::dom
{
  LiveNodeList::LiveNodeList(Node &root, LiveNodeListFilterFunc &&filter,
                             LiveCollectionInvalidation invalidation,
                             LiveCollectionTraversal traversal) noexcept
      : LiveCollection(root, invalidation, traversal), _filter(krys::move(filter))
  {
  }

  void LiveNodeList::InvalidateCache() noexcept
  {
    _indexCache.Invalidate();
    UnregisterCache();
  }

  void LiveNodeList::WillValidateIndexCache() const noexcept
  {
    RegisterCache();
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

  Node *LiveNodeList::CollectionBegin() const noexcept
  {
    return FirstMatchFrom(FirstNode());
  }

  Node *LiveNodeList::CollectionNext(Node &node) const noexcept
  {
    return FirstMatchFrom(NextNode(node));
  }

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
}
