#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"

namespace krys::boo::dom
{
  ContainerNode::ContainerNode(Document &document, dom::NodeType type, NodeFlags flags) noexcept
      : Node(document, type, flags | NodeFlags::IsContainerNode), _firstChild(nullptr), _lastChild(nullptr)
  {
  }
}
