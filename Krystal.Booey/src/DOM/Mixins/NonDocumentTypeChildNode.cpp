#include "Krystal.Booey/DOM/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"

namespace krys::boo::dom::Mixins
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