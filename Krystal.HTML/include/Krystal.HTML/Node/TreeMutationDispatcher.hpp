#pragma once

#include "Krystal.Lib/Pointers/ReferenceWrapper.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Node;
  class NodeList;

  struct NodeInsertedContext
  {
    ReferenceWrapper<ContainerNode> InsertedInto;
    bool ConnectedToDocument;
    bool TreeScopeChanged;
  };

  struct NodeRemovedContext
  {
    ReferenceWrapper<ContainerNode> RemovedFrom;
    bool DisconnectedFromDocument;
    bool TreeScopeChanged;
  };

  class TreeMutationDispatcher
  {
  public:
    static void NotifyNodeInserted(Node &node, ContainerNode &insertedInto) noexcept;
    static void NotifyNodeRemoved(Node &node, ContainerNode &removedFrom) noexcept;

    static void QueueMutationRecord(ContainerNode &target, Ref<NodeList> &&added, Ref<NodeList> &&removed,
                                    RefPtr<Node> &&previousSibling, RefPtr<Node> &&nextSibling) noexcept;

  private:
    static void DoNotifyNodeInserted(Node &node, const NodeInsertedContext &context) noexcept;
    static void DoNotifyNodeRemoved(Node &node, const NodeRemovedContext &context) noexcept;
  };
}