#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"

namespace Krys::HTML
{
  ContainerNode::ContainerNode(Document &document, HTML::NodeType type, NodeFlags flags) noexcept
      : Node(document, type, flags | NodeFlags::IsContainerNode), _firstChild(nullptr), _lastChild(nullptr)
  {
  }
}
