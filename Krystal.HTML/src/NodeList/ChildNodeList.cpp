#include "Krystal.HTML/NodeList/ChildNodeList.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Tree/TreeQueries.hpp"

namespace Krys::HTML
{
  ChildNodeList::ChildNodeList(Node &owner) noexcept
      : NodeList(NodeListType::ChildNode, NodeListFlag::None), _owner(ShareRef(owner)), _invalid(true)
  {
  }

  RawPtr<Node> ChildNodeList::Item(size_t index) const noexcept
  {
    if (_invalid)
    {
      BuildCollection();
    }

    if (index < _nodes.size())
    {
      return _nodes[index].get();
    }

    return nullptr;
  }

  size_t ChildNodeList::Length() const noexcept
  {
    if (_invalid)
    {
      BuildCollection();
    }

    return _nodes.size();
  }

  void ChildNodeList::BuildCollection() const noexcept
  {
    _invalid = false;

    if (auto *container = DynamicDowncast<ContainerNode>(*_owner))
    {
      _nodes.clear();
      TreeQueries::CollectChildNodes(*container, _nodes);
    }
  }
}