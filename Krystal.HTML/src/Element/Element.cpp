#include "Krystal.HTML/Element/Element.hpp"

namespace Krys::HTML
{
  Element::Element(Document &document, NodeFlag nodeFlags) noexcept
      : ContainerNode(document, NodeType::ELEMENT_NODE, nodeFlags | NodeFlag::IsElement)
  {
  }

  DOMString Element::NodeName() const noexcept
  {
    // TODO(IMPL): Return the qualified name
    return u8"element";
  }

  ExceptionOr<void> Element::RemoveAttributeNode(Attr &attribute) const noexcept
  {
    // TODO(IMPL): Implement this method
    return {};
  }
}