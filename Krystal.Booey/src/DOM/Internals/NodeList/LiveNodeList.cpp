#include "Krystal.Booey/DOM/Internals/NodeList/LiveNodeList.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::boo::dom
{
  LiveNodeList::LiveNodeList(Node &root, LiveNodeListFilterFunc &&filter) noexcept
      : _root(ShareRef(root)), _filter(krys::move(filter))
  {
  }

  RefPtr<Node> LiveNodeList::Item(size_t index) noexcept
  {
    size_t count = 0;
    for (Node &node : DescendantRange(*_root))
    {
      if (_filter(node))
      {
        ++count;
      }

      if (count > index)
      {
        return ShareRefPtr(&node);
      }
    }

    return nullptr;
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
    return Count(ConstDescendantRange(*_root), _filter);
  }
}
