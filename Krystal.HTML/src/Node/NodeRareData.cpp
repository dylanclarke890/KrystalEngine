#include "Krystal.HTML/Node/NodeRareData.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/NodeList/ChildNodeList.hpp"
#include "Krystal.HTML/NodeList/EmptyNodeList.hpp"

namespace Krys::HTML
{
  Ref<NodeList> NodeRareData::ChildNodes(Node &parent) noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(parent))
    {
      return CreateRef<ChildNodeList>(*containerNode);
    }

    return CreateRef<EmptyNodeList>(parent);
  }

  void NodeRareData::InvalidateChildNodes() noexcept
  {
    if (_childNodeList)
    {
      _childNodeList->Invalidate();
    }
  }
}
