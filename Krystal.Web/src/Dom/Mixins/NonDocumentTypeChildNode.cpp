#include "Krystal.Web/Dom/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/CharacterData.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Traversal/TreeTraversal.hpp"

namespace krys::web::dom::mixins
{
  RefPtr<Element> NonDocumentTypeChildNode::PreviousElementSibling(CharacterData &node) noexcept
  {
    return ShareRefPtr(TreeTraversal::PreviousElementSibling(node));
  }

  RefPtr<const Element> NonDocumentTypeChildNode::PreviousElementSibling(const CharacterData &node) noexcept
  {
    return ShareRefPtr(TreeTraversal::PreviousElementSibling(node));
  }

  RefPtr<Element> NonDocumentTypeChildNode::PreviousElementSibling(Element &node) noexcept
  {
    return ShareRefPtr(TreeTraversal::PreviousElementSibling(node));
  }

  RefPtr<const Element> NonDocumentTypeChildNode::PreviousElementSibling(const Element &node) noexcept
  {
    return ShareRefPtr(TreeTraversal::PreviousElementSibling(node));
  }

  RefPtr<Element> NonDocumentTypeChildNode::NextElementSibling(CharacterData &node) noexcept
  {
    return ShareRefPtr(TreeTraversal::NextElementSibling(node));
  }

  RefPtr<const Element> NonDocumentTypeChildNode::NextElementSibling(const CharacterData &node) noexcept
  {
    return ShareRefPtr(TreeTraversal::NextElementSibling(node));
  }

  RefPtr<Element> NonDocumentTypeChildNode::NextElementSibling(Element &node) noexcept
  {
    return ShareRefPtr(TreeTraversal::NextElementSibling(node));
  }

  RefPtr<const Element> NonDocumentTypeChildNode::NextElementSibling(const Element &node) noexcept
  {
    return ShareRefPtr(TreeTraversal::NextElementSibling(node));
  }
}
