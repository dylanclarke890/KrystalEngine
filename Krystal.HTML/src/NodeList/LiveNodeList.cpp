#include "Krystal.HTML/NodeList/LiveNodeList.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  LiveNodeList::LiveNodeList(Node &owner) noexcept
      : NodeList(NodeListType::Live, NodeListFlag::None), _owner(ShareRef(owner))
  {
  }
}
