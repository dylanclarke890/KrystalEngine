#include "Krystal.Booey/Dom/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.Booey/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/Dom/Nodes/Attr.hpp"
#include "Krystal.Booey/Dom/Nodes/CharacterData.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Dom/Nodes/ShadowRoot.hpp"

namespace krys::boo::dom::mixins
{
  RefPtr<Element> NonDocumentTypeChildNode::PreviousElementSibling(CharacterData &node) noexcept
  {
    return ShareRefPtr(TreeQueries::PreviousElementSibling(node));
  }

  RefPtr<const Element> NonDocumentTypeChildNode::PreviousElementSibling(const CharacterData &node) noexcept
  {
    return ShareRefPtr(TreeQueries::PreviousElementSibling(node));
  }

  RefPtr<Element> NonDocumentTypeChildNode::PreviousElementSibling(Element &node) noexcept
  {
    return ShareRefPtr(TreeQueries::PreviousElementSibling(node));
  }

  RefPtr<const Element> NonDocumentTypeChildNode::PreviousElementSibling(const Element &node) noexcept
  {
    return ShareRefPtr(TreeQueries::PreviousElementSibling(node));
  }

  RefPtr<Element> NonDocumentTypeChildNode::NextElementSibling(CharacterData &node) noexcept
  {
    return ShareRefPtr(TreeQueries::NextElementSibling(node));
  }

  RefPtr<const Element> NonDocumentTypeChildNode::NextElementSibling(const CharacterData &node) noexcept
  {
    return ShareRefPtr(TreeQueries::NextElementSibling(node));
  }

  RefPtr<Element> NonDocumentTypeChildNode::NextElementSibling(Element &node) noexcept
  {
    return ShareRefPtr(TreeQueries::NextElementSibling(node));
  }

  RefPtr<const Element> NonDocumentTypeChildNode::NextElementSibling(const Element &node) noexcept
  {
    return ShareRefPtr(TreeQueries::NextElementSibling(node));
  }
}
