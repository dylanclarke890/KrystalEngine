#include "Krystal.HTML/Algorithms/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include <cassert>

namespace Krys::HTML
{
  void TreeMutationDispatcher::QueueMutationRecord(
    DOMString &&type, Node &target, Maybe<DOMStringAtom> name, Maybe<DOMStringAtom> namespaceURI,
    Maybe<DOMStringView> oldValue, const SmallNodeList &addedNodes, const SmallNodeList &removedNodes,
    RefPtr<Node> &&previousSibling, RefPtr<Node> &&nextSibling) noexcept
  {
    // TODO: figure out the actual type to use here
    List<int> interestedObservers;

    auto nodes = TreeQueries::InclusiveAncestors(target);
    for (auto &node : nodes)
    {
    }
  }

  void TreeMutationDispatcher::QueueTreeMutationRecord(Node &target, const SmallNodeList &addedNodes,
                                                       const SmallNodeList &removedNodes,
                                                       RefPtr<Node> &&previousSibling,
                                                       RefPtr<Node> &&nextSibling) noexcept
  {
    assert(addedNodes.size() > 0 || removedNodes.size() > 0);
    QueueMutationRecord(u8"childList", target, std::nullopt, std::nullopt, std::nullopt, addedNodes,
                        removedNodes, Krys::Move(previousSibling), Krys::Move(nextSibling));
  }

  void TreeMutationDispatcher::LiveRangePreRemove(const Node &node) noexcept
  {
    auto *parent = node.ParentNode();
    assert(parent != nullptr);

    auto index = TreeQueries::Index(node);

    for (auto &range : node.NodeDocument().LiveRanges())
    {
      if (auto *startContainer = DynamicDowncast<ContainerNode>(*range->StartContainer()))
      {
        if (TreeQueries::IsInclusiveDescendant(*startContainer, node))
        {
          range->SetStart(*parent, index);
        }
      }

      if (auto *endContainer = DynamicDowncast<ContainerNode>(*range->EndContainer()))
      {
        if (TreeQueries::IsInclusiveDescendant(*endContainer, node))
        {
          range->SetEnd(*parent, index);
        }
      }

      if (range->StartContainer() == parent)
      {
        range->SetStart(*parent, range->StartOffset() - 1uz);
      }

      if (range->EndContainer() == parent)
      {
        range->SetEnd(*parent, range->EndOffset() - 1uz);
      }
    }
  }
}
