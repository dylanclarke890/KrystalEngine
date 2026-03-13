#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Utils/SmallNodeList.hpp"
#include "Krystal.Lib/Pointers/ReferenceWrapper.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Node;

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
    /// @see https://dom.spec.whatwg.org/#concept-node-insert-ext
    static void Inserted(ContainerNode &node, Node &insertedNode) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-post-connection-ext
    static void PostConnection(ContainerNode &node, Node &insertedNode) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-children-changed-ext
    static void ChildrenChanged(ContainerNode &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-move-ext
    static void Moved(ContainerNode &node, Node &movedNode, bool isSubtreeRoot,
                      ContainerNode &oldAncestor) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-remove-ext
    static void Removed(Node &removedNode, bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-clone-ext
    static void Cloned(Node &node, Node &copy, bool subtree) noexcept;

    /// @see https://dom.spec.whatwg.org/#queue-a-mutation-record
    static void QueueMutationRecord(DOMString &&type, Node &target, Maybe<DOMString> name,
                                    Maybe<DOMString> nameSpace, Maybe<DOMString> oldValue,
                                    const SmallNodeList &addedNodes, const SmallNodeList &removedNodes,
                                    RefPtr<Node> &&previousSibling, RefPtr<Node> &&nextSibling) noexcept;

    /// @see https://dom.spec.whatwg.org/#queue-a-tree-mutation-record
    static void QueueTreeMutationRecord(Node &target, const SmallNodeList &addedNodes,
                                        const SmallNodeList &removedNodes, RefPtr<Node> &&previousSibling,
                                        RefPtr<Node> &&nextSibling) noexcept;

    static void LiveRangePreRemove(const Node &node) noexcept;

  private:
    static void DispatchNodeInserted(Node &node, const NodeInsertedContext &context) noexcept;
    static void DispatchNodeRemoved(Node &node, const NodeRemovedContext &context) noexcept;
  };
}