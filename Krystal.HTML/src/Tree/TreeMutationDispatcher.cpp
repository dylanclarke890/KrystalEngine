#include "Krystal.HTML/Tree/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
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
