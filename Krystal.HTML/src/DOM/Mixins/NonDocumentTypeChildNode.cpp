#include "Krystal.HTML/DOM/Mixins/NonDocumentTypeChildNode.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML::Mixins
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