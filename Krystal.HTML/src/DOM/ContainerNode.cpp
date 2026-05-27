#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/Node/Document.hpp"

namespace Krys::HTML
{
  ContainerNode::ContainerNode(Document &document, HTML::NodeType type, NodeFlags flags) noexcept
      : Node(document, type, flags | NodeFlags::IsContainerNode), _firstChild(nullptr), _lastChild(nullptr)
  {
  }
}
