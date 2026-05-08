#include "Krystal.HTML/Iterator/NodeIterator.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  ExceptionOr<RefPtr<Node>> NodeIterator::NextNode() noexcept
  {
    return IteratorAlgorithms::Traverse(*this, TraversalNextOrPrev::Next);
  }

  ExceptionOr<RefPtr<Node>> NodeIterator::PreviousNode() noexcept
  {
    return IteratorAlgorithms::Traverse(*this, TraversalNextOrPrev::Previous);
  }
}