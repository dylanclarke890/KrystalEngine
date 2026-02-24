#include "Krystal.HTML/NodeList/EmptyNodeList.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Node.hpp"

namespace Krys::HTML
{
  EmptyNodeList::EmptyNodeList(Node &owner) noexcept
      : NodeList(NodeListType::Empty, NodeListFlag::None), _owner(ShareRef(owner))
  {
  }
}
