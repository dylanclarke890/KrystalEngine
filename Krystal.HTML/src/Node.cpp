#include "Krystal.HTML/Node.hpp"
#include "Krystal.HTML/ContainerNode.hpp"

namespace Krys::HTML
{
  bool Node::IsConnected() const noexcept
  {
    // TODO(IMPL): A node is connected if its shadow-including root(?) is a document.
    return false;
  }
}