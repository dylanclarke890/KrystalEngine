#include "Krystal.HTML/Iterator/NodeIterator.hpp"
#include "Krystal.HTML/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"


namespace Krys::HTML
{
  ExceptionOr<RefPtr<Node>> NodeIterator::NextNode() noexcept
  {
    return IteratorAlgorithms::Traverse(*this, TraversalType::Next);
  }
  
  ExceptionOr<RefPtr<Node>> NodeIterator::PreviousNode() noexcept
  {
    return IteratorAlgorithms::Traverse(*this, TraversalType::Previous);
  }
}