#include "Krystal.Booey/DOM/Internals/NodeList/StaticNodeList.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::boo::dom
{
  StaticNodeList::StaticNodeList(SmallNodeList &&nodes) noexcept : _nodes(krys::move(nodes))
  {
  }

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
}
