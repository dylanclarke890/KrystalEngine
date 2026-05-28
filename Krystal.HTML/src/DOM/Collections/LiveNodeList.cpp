#include "Krystal.HTML/DOM/Collections/LiveNodeList.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  LiveNodeList::LiveNodeList(Node &root, LiveNodeListFilterFunc &&filter) noexcept
      : _root(ShareRef(root)), _filter(Krys::Move(filter))
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
