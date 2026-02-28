#include "Krystal.HTML/Node/NodeRareData.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/NodeList/ChildNodeList.hpp"

namespace Krys::HTML
{
  Ref<NodeList> NodeRareData::ChildNodes(Node &node) noexcept
  {
    if (!_childNodeList)
    {
      auto children = CreateRef<ChildNodeList>(node);
      _childNodeList = CreateWeakPtr<NodeList>(children.get());
      return children;
    }

    return ShareRef(*_childNodeList.get());
  }

  void NodeRareData::InvalidateChildNodes() noexcept
  {
    if (_childNodeList)
    {
      _childNodeList->Invalidate();
    }
  }
}
