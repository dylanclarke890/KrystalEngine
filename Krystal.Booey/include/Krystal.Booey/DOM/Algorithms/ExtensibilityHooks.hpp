#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
  class Node;

  /// @see https://dom.spec.whatwg.org/#other-applicable-specifications
  class ExtensibilityHooks
  {
  public:
    /// @param isSubtreeRoot Whether `insertedNode` is the node that was inserted, rather than one of its
    /// shadow-including descendants.
    /// @see https://dom.spec.whatwg.org/#concept-node-insert-ext
    static void NodeInserted(Node &insertedNode, bool isSubtreeRoot) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-post-connection-ext
    static void NodePostConnection(Node &connectedNode) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-children-changed-ext
    static void NodeChildrenChanged(ContainerNode &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-move-ext
    static void NodeMoved(Node &movedNode, bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-remove-ext
    static void NodeRemoved(Node &removedNode, bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept;

    /// @brief Destroys the renderers of @p child, which is about to be removed or moved. It's still in the
    /// tree, so renderers that depend on their place (list items, counters) can update their neighbours.
    static void DestroyRenderTreeIfNeeded(Node &child) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-clone-ext
    static void NodeCloned(Node &node, Node &copy, bool subtree) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-adopt-ext
    static void NodeAdopted(Node &node, Document &oldDocument) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-change-ext
    static void ElementAttributeChanged(Element &element, DOMStringAtom localName,
                                        Maybe<DOMStringView> oldValue, Maybe<DOMStringView> value,
                                        DOMStringAtom namespaceURI) noexcept;
  };
}