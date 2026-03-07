#include "Krystal.HTML/Tree/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"
#include "Krystal.HTML/Tree/TreeQueries.hpp"
#include "Krystal.HTML/Tree/TreeTraversal.hpp"
#include <cassert>

namespace Krys::HTML
{
  void TreeMutationDispatcher::NodeInserted(Node &node, ContainerNode &insertedInto) noexcept
  {
    assert(!node.IsConnected());

    ChildrenChanged(insertedInto);

    NodeInsertedContext context {
      .InsertedInto = insertedInto,
      .ConnectedToDocument = insertedInto.IsConnected(),
      .TreeScopeChanged = insertedInto.IsInTreeScope(), // TODO(FIX): More precise check
    };

    DispatchNodeInserted(node, context);
  }

  void TreeMutationDispatcher::NodeRemoved(Node &node, ContainerNode &removedFrom) noexcept
  {
    ChildrenChanged(removedFrom);

    NodeRemovedContext context {
      .RemovedFrom = removedFrom,
      .DisconnectedFromDocument = node.IsConnected(),
      .TreeScopeChanged = removedFrom.IsInTreeScope(), // TODO(FIX): More precise check
    };

    DispatchNodeRemoved(node, context);
  }

  void TreeMutationDispatcher::ChildrenChanged(ContainerNode &node) noexcept
  {
    node.OnChildrenChanged();
  }

  void TreeMutationDispatcher::NodeCloned(Node &node, Node &copy, bool subtree) noexcept
  {
    (void)node;
    (void)copy;
    (void)subtree;

    // TODO(IMPL): Run any cloning steps defined for node in other applicable specifications and pass node,
    // copy, and subtree as parameters.
  }

  void TreeMutationDispatcher::QueueMutationRecord(DOMString &&type, Node &target, Maybe<DOMString> name,
                                                   Maybe<DOMString> nameSpace, Maybe<DOMString> oldValue,
                                                   Ref<NodeList> &&addedNodes, Ref<NodeList> &&removedNodes,
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

  void TreeMutationDispatcher::QueueTreeMutationRecord(Node &target, Ref<NodeList> &&addedNodes,
                                                       Ref<NodeList> &&removedNodes,
                                                       RefPtr<Node> &&previousSibling,
                                                       RefPtr<Node> &&nextSibling) noexcept
  {
    assert(addedNodes->Length() > 0 || removedNodes->Length() > 0);
    QueueMutationRecord(u8"childList", target, std::nullopt, std::nullopt, std::nullopt,
                        Krys::Move(addedNodes), Krys::Move(removedNodes), Krys::Move(previousSibling),
                        Krys::Move(nextSibling));
  }

  void TreeMutationDispatcher::LiveRangePreRemove(const Node &node) noexcept
  {
    auto *parent = node.ParentNode();
    assert(parent != nullptr);

    auto index = TreeQueries::NodeIndex(node);

    for (auto &range : node.NodeDocument()._liveRanges)
    {
      if (auto *startContainer = DynamicDowncast<ContainerNode>(*range->StartContainer()))
      {
        if (TreeQueries::IsInclusiveDescendantOf(*startContainer, node))
        {
          range->SetStart(*parent, index);
        }
      }

      if (auto *endContainer = DynamicDowncast<ContainerNode>(*range->EndContainer()))
      {
        if (TreeQueries::IsInclusiveDescendantOf(*endContainer, node))
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

  void TreeMutationDispatcher::DispatchNodeInserted(Node &node, const NodeInsertedContext &context) noexcept
  {
    for (auto *currentNode = &node; currentNode; currentNode = TreeTraversal::Next(*currentNode, &node))
    {
      currentNode->InsertedIntoAncestor(context);
      if (RawPtr<ShadowRoot> root = TreeQueries::GetShadowRoot(*currentNode))
      {
        NodeInsertedContext ctx = {
          .InsertedInto = context.InsertedInto,
          .ConnectedToDocument = context.ConnectedToDocument,
          .TreeScopeChanged = false, // The tree scope of a shadow root doesn't change on node insertion
        };

        DispatchNodeInserted(*root, ctx);
      }
    }
  }

  void TreeMutationDispatcher::DispatchNodeRemoved(Node &node, const NodeRemovedContext &context) noexcept
  {
    for (auto *currentNode = &node; currentNode; currentNode = TreeTraversal::Next(*currentNode))
    {
      currentNode->RemovedFromAncestor(context);
      if (RawPtr<ShadowRoot> root = TreeQueries::GetShadowRoot(*currentNode))
      {
        NodeRemovedContext ctx = {
          .RemovedFrom = context.RemovedFrom,
          .DisconnectedFromDocument = context.DisconnectedFromDocument,
          .TreeScopeChanged = false, // The tree scope of a shadow root doesn't change on node removal
        };

        DispatchNodeRemoved(*root, ctx);
      }
    }
  }
}
