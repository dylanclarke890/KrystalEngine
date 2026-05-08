#pragma once

#include "Krystal.HTML/DOMString.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Element;
  class Node;

  /// @see https://dom.spec.whatwg.org/#other-applicable-specifications
  class ExtensibilityHooks
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-node-insert-ext
    static void Inserted(Node &insertedNode) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-post-connection-ext
    static void PostConnection(Node &connectedNode) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-children-changed-ext
    static void ChildrenChanged(ContainerNode &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-move-ext
    static void Moved(Node &node, Node &movedNode, bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-remove-ext
    static void Removed(Node &removedNode, bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-clone-ext
    static void Cloned(const Node &node, Node &copy, bool subtree) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-change-ext
    static void AttributeChanged(Element &element, DOMStringAtom localName, DOMStringView oldValue,
                                 DOMStringView value, DOMStringAtom namespaceURI) noexcept;
  };
}