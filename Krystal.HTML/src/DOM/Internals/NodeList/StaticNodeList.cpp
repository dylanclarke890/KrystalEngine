#include "Krystal.HTML/DOM/Internals/NodeList/StaticNodeList.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  StaticNodeList::StaticNodeList(SmallNodeList &&nodes) noexcept : _nodes(Krys::Move(nodes))
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
