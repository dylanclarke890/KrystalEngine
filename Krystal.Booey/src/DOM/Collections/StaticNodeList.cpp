#include "Krystal.Booey/DOM/Collections/StaticNodeList.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Core/Move.hpp"

namespace krys::boo
{
  dom::StaticNodeList::StaticNodeList(dom::SmallNodeList &&nodes) noexcept : _nodes(krys::move(nodes))
  {
  }

  RefPtr<Node> dom::StaticNodeList::Item(size_t index) noexcept
  {
    if (IsSupportedPropertyIndex(index))
    {
      return _nodes[index];
    }

    return nullptr;
  }

  RefPtr<const Node> dom::StaticNodeList::Item(size_t index) const noexcept
  {
    return const_cast<dom::StaticNodeList *>(this)->Item(index);
  }

  RefPtr<Node> dom::StaticNodeList::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<const Node> dom::StaticNodeList::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  size_t dom::StaticNodeList::Length() const noexcept
  {
    return _nodes.size();
  }
}
