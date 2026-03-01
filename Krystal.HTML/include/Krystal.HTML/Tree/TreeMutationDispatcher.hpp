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
    static void NodeInserted(Node &node, ContainerNode &insertedInto) noexcept;
    static void NodeRemoved(Node &node, ContainerNode &removedFrom) noexcept;
    static void ChildrenChanged(ContainerNode &node) noexcept;

    // static void QueueMutationRecord(ContainerNode &target, Ref<NodeList> &&added, Ref<NodeList> &&removed,
    //                                 RefPtr<Node> &&previousSibling, RefPtr<Node> &&nextSibling) noexcept;

  private:
    static void DispatchNodeInserted(Node &node, const NodeInsertedContext &context) noexcept;
    static void DispatchNodeRemoved(Node &node, const NodeRemovedContext &context) noexcept;
  };
}