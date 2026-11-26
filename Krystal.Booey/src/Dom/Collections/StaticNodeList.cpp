#include "Krystal.Booey/Dom/Collections/StaticNodeList.hpp"
#include "Krystal.Core/Move.hpp"

namespace krys::boo
{
  StaticNodeList::StaticNodeList(dom::SmallNodeList nodes) noexcept : _nodes(krys::move(nodes))
  {
  }

#pragma region NodeList - https://dom.spec.whatwg.org/#nodelist

  RefPtr<Node> StaticNodeList::Item(size_t index) noexcept
  {
    if (IsSupportedPropertyIndex(index))
    {
      return _nodes[index];
    }

    return nullptr;
  }

  RefPtr<const Node> StaticNodeList::Item(size_t index) const noexcept
  {
    return const_cast<StaticNodeList *>(this)->Item(index);
  }

  RefPtr<Node> StaticNodeList::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RefPtr<const Node> StaticNodeList::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  size_t StaticNodeList::Length() const noexcept
  {
    return _nodes.size();
  }

#pragma endregion
}
