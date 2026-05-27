#include "Krystal.HTML/Iterator/NodeIterator.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  NodeIterator::NodeIterator(Node &root, HTML::WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept
      : IteratorBase(root, whatToShow, Krys::Move(filter)), _referenceNode(ShareRef(root)),
        _pointerBeforeReferenceNode(true)
  {
  }

  ExceptionOr<RefPtr<Node>> NodeIterator::NextNode() noexcept
  {
    return IteratorAlgorithms::Traverse(*this, TraversalNextOrPrev::Next);
  }

  ExceptionOr<RefPtr<Node>> NodeIterator::PreviousNode() noexcept
  {
    return IteratorAlgorithms::Traverse(*this, TraversalNextOrPrev::Previous);
  }
}