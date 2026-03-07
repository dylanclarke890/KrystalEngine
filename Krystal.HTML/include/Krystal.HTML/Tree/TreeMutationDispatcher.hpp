#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.Lib/Pointers/ReferenceWrapper.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

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
    static void NodeCloned(Node &node, Node &copy, bool subtree) noexcept;

    /// @see https://dom.spec.whatwg.org/#queue-a-mutation-record
    static void QueueMutationRecord(DOMString &&type, Node &target, Maybe<DOMString> name,
                                    Maybe<DOMString> nameSpace, Maybe<DOMString> oldValue,
                                    Ref<NodeList> &&addedNodes, Ref<NodeList> &&removedNodes,
                                    RefPtr<Node> &&previousSibling, RefPtr<Node> &&nextSibling) noexcept;

    /// @see https://dom.spec.whatwg.org/#queue-a-tree-mutation-record
    static void QueueTreeMutationRecord(Node &target, Ref<NodeList> &&addedNodes,
                                        Ref<NodeList> &&removedNodes, RefPtr<Node> &&previousSibling,
                                        RefPtr<Node> &&nextSibling) noexcept;

    static void LiveRangePreRemove(const Node &node) noexcept;

  private:
    static void DispatchNodeInserted(Node &node, const NodeInsertedContext &context) noexcept;
    static void DispatchNodeRemoved(Node &node, const NodeRemovedContext &context) noexcept;
  };
}