#pragma once

#include "Krystal.HTML/DOM/Types/ExceptionOr.hpp"
#include "Krystal.HTML/DOM/Types/NodeOrString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Document;
  class Element;
  class HTMLCollection;
  class Node;
  class NodeList;
}

namespace Krys::HTML::Mixins
{
  /// @see https://dom.spec.whatwg.org/#parentnode
  class ParentNode
  {
  public:
    /// @brief Converts a list of nodes or strings into a single node, which is either the single node in the
    /// list or a DocumentFragment containing all the nodes in the list. If any of the items in the list are
    /// strings, they are converted to Text nodes using the given document.
    /// @see https://dom.spec.whatwg.org/#convert-nodes-into-a-node
    KRYS_NODISCARD static ExceptionOr<Ref<Node>> ConvertNodesIntoNode(const List<NodeOrString> &nodes,
                                                                      Document &document) noexcept;

    /// @brief Returns the first child that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-firstelementchild
    KRYS_NODISCARD static RefPtr<Element> FirstElementChild(ContainerNode &node) noexcept;

    /// @brief Returns the first child that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-firstelementchild
    KRYS_NODISCARD static RefPtr<const Element> FirstElementChild(const ContainerNode &node) noexcept;

    /// @brief Returns the last child that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-lastelementchild
    KRYS_NODISCARD static RefPtr<Element> LastElementChild(ContainerNode &node) noexcept;

    /// @brief Returns the last child that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-lastelementchild
    KRYS_NODISCARD static RefPtr<const Element> LastElementChild(const ContainerNode &node) noexcept;

    /// @brief Returns the number of child elements.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-childelementcount
    KRYS_NODISCARD static size_t ChildElementCount(const ContainerNode &node) noexcept;

    /// @brief Inserts `nodes` before the first child of `node`, while replacing strings in `nodes` with
    /// equivalent Text nodes.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-prepend
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    KRYS_NODISCARD static ExceptionOr<void> Prepend(ContainerNode &node,
                                                    const List<NodeOrString> &nodes) noexcept;

    /// @brief Inserts `nodes` after the last child of `node`, while replacing strings in `nodes` with
    /// equivalent Text nodes.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-append
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    KRYS_NODISCARD static ExceptionOr<void> Append(ContainerNode &parent,
                                                   const List<NodeOrString> &nodes) noexcept;

    /// @brief Replace all children of `node` with `nodes`, while replacing strings in `nodes` with equivalent
    /// Text nodes.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-replacechildren
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    KRYS_NODISCARD static ExceptionOr<void> ReplaceChildren(ContainerNode &node,
                                                            const List<NodeOrString> &nodes) noexcept;

    /// @brief Moves, without first removing, `movedNode` into `node` after `child` if `child` is non-null;
    /// otherwise after the last child of node. This method preserves state associated with `movedNode`.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-movebefore
    /// @throws HierarchyRequestError if the constraints of the node tree are violated, or the state
    /// associated with the moved node cannot be preserved.
    KRYS_NODISCARD static ExceptionOr<void> MoveBefore(ContainerNode &node, Node &movedNode,
                                                       RawPtr<Node> child) noexcept;

    /// @brief Returns the first element that is a descendant of `node` that matches `selectors`.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-queryselector
    KRYS_NODISCARD static ExceptionOr<RefPtr<Element>> QuerySelector(ContainerNode &node,
                                                                     DOMStringView selectors) noexcept;

    /// @brief Returns all element descendants of `node` that match `selectors`.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-queryselectorall
    KRYS_NODISCARD static ExceptionOr<Ref<NodeList>> QuerySelectorAll(ContainerNode &node,
                                                                      DOMStringView selectors) noexcept;
  };
}