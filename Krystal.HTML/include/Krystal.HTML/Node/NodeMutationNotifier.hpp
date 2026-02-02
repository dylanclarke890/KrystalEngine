#pragma once

namespace Krys::HTML
{
  struct NodeInsertedContext;
  struct NodeRemovedContext;
  class ContainerNode;
  class Node;

  class NodeMutationNotifier
  {
  public:
    static void NotifyNodeInserted(Node &node, ContainerNode &insertedInto) noexcept;
    static void NotifyNodeRemoved(Node &node, ContainerNode &removedFrom) noexcept;

  private:
    static void DoNotifyNodeInserted(Node &node, const NodeInsertedContext &context) noexcept;
    static void DoNotifyNodeRemoved(Node &node, const NodeRemovedContext &context) noexcept;
  };
}