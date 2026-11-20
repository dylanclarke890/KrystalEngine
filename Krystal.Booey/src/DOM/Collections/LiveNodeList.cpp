#include "Krystal.Booey/DOM/Collections/LiveNodeList.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::boo
{
  dom::LiveNodeList::LiveNodeList(Node &root, dom::LiveNodeListFilterFunc &&filter,
                                  dom::LiveCollectionInvalidation invalidation,
                                  dom::LiveCollectionTraversal traversal) noexcept
      : dom::LiveCollection(root, invalidation, traversal), _filter(krys::move(filter))
  {
  }

  void dom::LiveNodeList::InvalidateCache() noexcept
  {
    _indexCache.Invalidate();
    UnregisterCache();
  }

  void dom::LiveNodeList::WillValidateIndexCache() const noexcept
  {
    RegisterCache();
  }

  Node *dom::LiveNodeList::FirstMatchFrom(Node *node) const noexcept
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

  Node *dom::LiveNodeList::CollectionBegin() const noexcept
  {
    return FirstMatchFrom(FirstNode());
  }

  Node *dom::LiveNodeList::CollectionNext(Node &node) const noexcept
  {
    return FirstMatchFrom(NextNode(node));
  }

  RefPtr<Node> dom::LiveNodeList::Item(size_t index) noexcept
  {
    return ShareRefPtr(_indexCache.At(*this, index));
  }

  RefPtr<const Node> dom::LiveNodeList::Item(size_t index) const noexcept
  {
    return const_cast<dom::LiveNodeList *>(this)->Item(index);
  }

  RefPtr<Node> dom::LiveNodeList::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<const Node> dom::LiveNodeList::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  size_t dom::LiveNodeList::Length() const noexcept
  {
    return _indexCache.Count(*this);
  }
}
