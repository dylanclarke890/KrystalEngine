#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Lib/Core/Move.hpp"

namespace Krys::HTML
{
  LiveNodeList::LiveNodeList(WeakRef<Node> &&root, LiveNodeListFilterFunc &&filter) noexcept
      : NodeList(NodeCollectionLiveness::Live), _root(Krys::Move(root)), _filter(Krys::Move(filter))
  {
  }

  RawPtr<Node> LiveNodeList::Item(size_t index) noexcept
  {
    if (auto root = _root.lock())
    {
      size_t count = 0;
      for (Node &node : DescendantRange(*root))
      {
        if (_filter(node))
        {
          ++count;
        }

        if (count > index)
        {
          return &node;
        }
      }
    }

    return nullptr;
  }

  RawPtr<const Node> LiveNodeList::Item(size_t index) const noexcept
  {
    if (auto root = _root.lock())
    {
      size_t count = 0;
      for (const Node &node : ConstDescendantRange(*root))
      {
        if (_filter(node))
        {
          ++count;
        }

        if (count > index)
        {
          return &node;
        }
      }
    }

    return nullptr;
  }

  RawPtr<Node> LiveNodeList::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RawPtr<const Node> LiveNodeList::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  size_t LiveNodeList::Length() const noexcept
  {
    if (auto root = _root.lock())
    {
      return Count(ConstDescendantRange(*root), _filter);
    }

    return 0uz;
  }

  StaticNodeList::StaticNodeList(SmallNodeList &&nodes) noexcept
      : NodeList(NodeCollectionLiveness::Static), _nodes(Krys::Move(nodes))
  {
  }

  RawPtr<Node> StaticNodeList::Item(size_t index) noexcept
  {
    if (index < _nodes.size())
    {
      return _nodes[index].get();
    }

    return nullptr;
  }

  RawPtr<const Node> StaticNodeList::Item(size_t index) const noexcept
  {
    if (index < _nodes.size())
    {
      return _nodes[index].get();
    }

    return nullptr;
  }

  RawPtr<Node> StaticNodeList::operator[](size_t index) noexcept
  {
    return Item(index);
  }

  RawPtr<const Node> StaticNodeList::operator[](size_t index) const noexcept
  {
    return Item(index);
  }

  size_t StaticNodeList::Length() const noexcept
  {
    return _nodes.size();
  }
}
