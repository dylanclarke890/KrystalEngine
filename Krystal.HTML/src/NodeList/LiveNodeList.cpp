#include "Krystal.HTML/NodeList/LiveNodeList.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Node.hpp"

namespace Krys::HTML
{
  LiveNodeList::LiveNodeList(Node &owner) noexcept
      : NodeList(NodeListType::Live, NodeListFlag::None), _owner(ShareRef(owner))
  {
  }
}
