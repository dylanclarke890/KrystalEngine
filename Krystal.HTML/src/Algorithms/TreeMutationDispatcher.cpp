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
#include "Krystal.HTML/NodeList/NodeList.hpp"
#include <cassert>

namespace Krys::HTML
{
  void TreeMutationDispatcher::Inserted(Node &insertedNode) noexcept
  {
    auto *parent = insertedNode.ParentNode();
    if (parent == nullptr)
    {
      return;
    }

    if (parent->IsConnected())
    {
      insertedNode.SetEventTargetFlag(EventTargetFlag::IsConnected);
    }

    if (parent->IsInShadowTree())
    {
      insertedNode.SetEventTargetFlag(EventTargetFlag::IsInShadowTree);
    }
  }

  void TreeMutationDispatcher::PostConnection(Node &connectedNode) noexcept
  {
    // TODO(impl): post connection steps
  }

  void TreeMutationDispatcher::ChildrenChanged(ContainerNode &node) noexcept
  {
    node.InvalidateChildNodes();
    node.OnChildrenChanged();
  }

  void TreeMutationDispatcher::Moved(ContainerNode &node, Node &movedNode, bool isSubtreeRoot,
                                     ContainerNode &oldAncestor) noexcept
  {
  }

  void TreeMutationDispatcher::Removed(Node &removedNode, bool isSubtreeRoot,
                                       ContainerNode &oldAncestor) noexcept
  {
    removedNode.ClearEventTargetFlag(EventTargetFlag::IsConnected);
    removedNode.ClearEventTargetFlag(EventTargetFlag::IsInShadowTree);
  }

  void TreeMutationDispatcher::Cloned(Node &node, Node &copy, bool subtree) noexcept
  {
    // TODO(IMPL): Run any cloning steps defined for node in other applicable specifications and pass node,
    // copy, and subtree as parameters.
  }

  void TreeMutationDispatcher::QueueMutationRecord(DOMString &&type, Node &target, Maybe<DOMString> name,
                                                   Maybe<DOMString> nameSpace, Maybe<DOMString> oldValue,
                                                   const SmallNodeList &addedNodes,
                                                   const SmallNodeList &removedNodes,
                                                   RefPtr<Node> &&previousSibling,
                                                   RefPtr<Node> &&nextSibling) noexcept
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
