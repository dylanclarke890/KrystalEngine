#include "Krystal.HTML/Node/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Document/ShadowRoot.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeTraversal.hpp"
#include <cassert>

namespace Krys::HTML
{
  void TreeMutationDispatcher::DoNotifyNodeInserted(Node &node, const NodeInsertedContext &context) noexcept
  {
    for (RawPtr<Node> currentNode = &node; currentNode != nullptr;
         currentNode = NodeTraversal::Next(*currentNode, &node))
    {
      currentNode->InsertedIntoAncestor(context);
      if (RawPtr<ShadowRoot> root = currentNode->GetShadowRoot())
      {
        NodeInsertedContext ctx = {
          .InsertedInto = context.InsertedInto,
          .ConnectedToDocument = context.ConnectedToDocument,
          .TreeScopeChanged = false, // The tree scope of a shadow root doesn't change on node insertion
        };
        DoNotifyNodeInserted(*root, ctx);
      }
    }
  }

  void TreeMutationDispatcher::DoNotifyNodeRemoved(Node &node, const NodeRemovedContext &context) noexcept
  {
    for (RawPtr<Node> currentNode = &node; currentNode != nullptr;
         currentNode = NodeTraversal::Next(*currentNode))
    {
      currentNode->RemovedFromAncestor(context);
      if (RawPtr<ShadowRoot> root = currentNode->GetShadowRoot())
      {
        NodeRemovedContext ctx = {
          .RemovedFrom = context.RemovedFrom,
          .DisconnectedFromDocument = context.DisconnectedFromDocument,
          .TreeScopeChanged = false, // The tree scope of a shadow root doesn't change on node removal
        };
        DoNotifyNodeRemoved(*root, ctx);
      }
    }
  }

  void TreeMutationDispatcher::NotifyNodeInserted(Node &node, ContainerNode &insertedInto) noexcept
  {
    assert(!node.IsConnected());

    NodeInsertedContext context {
      .InsertedInto = insertedInto,
      .ConnectedToDocument = insertedInto.IsConnected(),
      .TreeScopeChanged = insertedInto.IsInTreeScope(), // TODO(FIX): More precise check
    };
    DoNotifyNodeInserted(node, context);
  }

  void TreeMutationDispatcher::NotifyNodeRemoved(Node &node, ContainerNode &removedFrom) noexcept
  {
    NodeRemovedContext context {
      .RemovedFrom = removedFrom,
      .DisconnectedFromDocument = node.IsConnected(),
      .TreeScopeChanged = removedFrom.IsInTreeScope(), // TODO(FIX): More precise check
    };
    DoNotifyNodeRemoved(node, context);
  }
}
