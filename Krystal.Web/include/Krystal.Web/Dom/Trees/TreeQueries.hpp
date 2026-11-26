#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::web
{
  class ContainerNode;
  class Document;
  class Element;
  class Node;
  class ShadowRoot;
}

namespace krys::web::dom
{
  /// @brief Implementations of the tree concepts and algorithms defined in the DOM spec. The engine's own
  /// tree helpers are in TreeTraversal.
  /// @see https://dom.spec.whatwg.org/#trees
  /// @see https://dom.spec.whatwg.org/#node-trees
  /// @see https://dom.spec.whatwg.org/#document-trees
  /// @see https://dom.spec.whatwg.org/#shadow-trees
  /// @see https://dom.spec.whatwg.org/#interface-documentfragment
  class TreeQueries
  {
  public:

#pragma region Trees - https://dom.spec.whatwg.org/#trees

    /// @brief Whether @p a is @p b's parent.
    /// @see https://dom.spec.whatwg.org/#concept-tree-parent
    KRYS_NODISCARD static bool IsParent(const Node &a, const Node &b) noexcept;

    /// @brief Whether @p a is a child of @p b.
    /// @see https://dom.spec.whatwg.org/#concept-tree-child
    KRYS_NODISCARD static bool IsChild(const Node &a, const Node &b) noexcept;

    /// @brief @p node itself if its parent is null, or else the root of its parent.
    /// @see https://dom.spec.whatwg.org/#concept-tree-root
    KRYS_NODISCARD static const Node &Root(const Node &node) noexcept;

    /// @brief @p node itself if its parent is null, or else the root of its parent.
    /// @see https://dom.spec.whatwg.org/#concept-tree-root
    KRYS_NODISCARD static Node &Root(Node &node) noexcept;

    /// @brief Whether @p a is a descendant of @p b.
    /// @see https://dom.spec.whatwg.org/#concept-tree-descendant
    KRYS_NODISCARD static bool IsDescendant(const Node &a, const Node &b) noexcept;

    /// @brief Whether @p b isn't null and @p a is a descendant of it.
    /// @see https://dom.spec.whatwg.org/#concept-tree-descendant
    KRYS_NODISCARD static bool IsDescendant(const Node &a, const Node *b) noexcept;

    /// @brief Whether @p a is @p b or one of its descendants.
    /// @see https://dom.spec.whatwg.org/#concept-tree-inclusive-descendant
    KRYS_NODISCARD static bool IsInclusiveDescendant(const Node &a, const Node &b) noexcept;

    /// @brief Whether @p a is an ancestor of @p b.
    /// @see https://dom.spec.whatwg.org/#concept-tree-ancestor
    KRYS_NODISCARD static bool IsAncestor(const Node &a, const Node &b) noexcept;

    /// @brief Whether @p a is @p b or one of its ancestors.
    /// @see https://dom.spec.whatwg.org/#concept-tree-inclusive-ancestor
    KRYS_NODISCARD static bool IsInclusiveAncestor(const Node &a, const Node &b) noexcept;

    /// @brief Whether @p a and @p b are different nodes that share the same non-null parent.
    /// @see https://dom.spec.whatwg.org/#concept-tree-sibling
    KRYS_NODISCARD static bool IsSibling(const Node &a, const Node &b) noexcept;

    /// @brief Whether @p a is @p b or one of its siblings.
    /// @see https://dom.spec.whatwg.org/#concept-tree-inclusive-sibling
    KRYS_NODISCARD static bool IsInclusiveSibling(const Node &a, const Node &b) noexcept;

    /// @brief Whether @p a and @p b are in the same tree and @p a comes before @p b in tree order.
    /// @see https://dom.spec.whatwg.org/#concept-tree-preceding
    KRYS_NODISCARD static bool IsPreceding(const Node &a, const Node &b) noexcept;

    /// @brief Whether @p a and @p b are in the same tree and @p a comes after @p b in tree order.
    /// @see https://dom.spec.whatwg.org/#concept-tree-following
    KRYS_NODISCARD static bool IsFollowing(const Node &a, const Node &b) noexcept;

    /// @brief The number of @p node's preceding siblings.
    /// @see https://dom.spec.whatwg.org/#concept-tree-index
    KRYS_NODISCARD static size_t Index(const Node &node) noexcept;

#pragma endregion

#pragma region Node Trees - https://dom.spec.whatwg.org/#node-trees

    /// @see https://dom.spec.whatwg.org/#concept-node-length
    KRYS_NODISCARD static size_t Length(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-empty
    KRYS_NODISCARD static bool IsEmpty(const Node &node) noexcept;

#pragma endregion

#pragma region Document Trees - https://dom.spec.whatwg.org/#document-trees

    /// @see https://dom.spec.whatwg.org/#in-a-document-tree
    KRYS_NODISCARD static bool IsInDocumentTree(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#document-element
    KRYS_NODISCARD static const Element *DocumentElement(const Document &document) noexcept;

    /// @see https://dom.spec.whatwg.org/#document-element
    KRYS_NODISCARD static Element *DocumentElement(Document &document) noexcept;

#pragma endregion

#pragma region Shadow Trees - https://dom.spec.whatwg.org/#shadow-trees

    /// @see https://dom.spec.whatwg.org/#concept-shadow-tree
    KRYS_NODISCARD static bool IsInShadowTree(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-host-including-inclusive-ancestor
    KRYS_NODISCARD static bool IsHostIncludingInclusiveAncestorOf(const Node &a, const Node &b) noexcept;

    /// @brief Whether @p node is a DocumentFragment node that isn't a ShadowRoot node.
    /// @see https://dom.spec.whatwg.org/#exclusive-documentfragment-node
    KRYS_NODISCARD static bool IsExclusiveDocumentFragmentNode(const Node &node) noexcept;

#pragma endregion

#pragma region Shadow Roots

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-root
    KRYS_NODISCARD static const Node &ShadowIncludingRoot(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-root
    KRYS_NODISCARD static Node &ShadowIncludingRoot(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-descendant
    KRYS_NODISCARD static bool IsShadowIncludingDescendant(const Node &a, const Node &b) noexcept;

    /// @brief Whether @p b isn't null and @p a is a shadow-including descendant of it.
    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-descendant
    KRYS_NODISCARD static bool IsShadowIncludingDescendant(const Node &a, const Node *b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-inclusive-descendant
    KRYS_NODISCARD static bool IsShadowIncludingInclusiveDescendant(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-ancestor
    KRYS_NODISCARD static bool IsShadowIncludingAncestor(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-inclusive-ancestor
    KRYS_NODISCARD static bool IsShadowIncludingInclusiveAncestor(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-closed-shadow-hidden
    KRYS_NODISCARD static bool IsClosedShadowHidden(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#element-shadow-host
    KRYS_NODISCARD static bool IsShadowHost(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#element-shadow-host
    KRYS_NODISCARD static bool IsShadowHost(const Element &node) noexcept;

#pragma endregion

#pragma region Tree Order Traversal

    /// @brief The node after @p current in tree order (preorder, depth-first), or null if it's the last.
    /// @see https://dom.spec.whatwg.org/#concept-tree-order
    KRYS_NODISCARD static Node *Next(const Node &current) noexcept;

    /// @brief The node after @p current in tree order (preorder, depth-first), or null if it's the last.
    /// @see https://dom.spec.whatwg.org/#concept-tree-order
    KRYS_NODISCARD static Node *Next(const ContainerNode &current) noexcept;

    /// @brief The node after @p current in tree order, or null if that isn't a descendant of @p stayWithin.
    /// @see https://dom.spec.whatwg.org/#concept-tree-order
    KRYS_NODISCARD static Node *Next(const Node &current, const Node *stayWithin) noexcept;

    /// @brief The node after @p current in tree order, or null if that isn't a descendant of @p stayWithin.
    /// @see https://dom.spec.whatwg.org/#concept-tree-order
    KRYS_NODISCARD static Node *Next(const ContainerNode &current, const Node *stayWithin) noexcept;

    /// @brief The next node in shadow-including tree order: a host's shadow root comes just after it.
    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-tree-order
    /// @see https://dom.spec.whatwg.org/#shadow-including-preorder-depth-first-traversal
    KRYS_NODISCARD static Node *NextShadowIncluding(const Node &current) noexcept;

    /// @brief Helper function for getting the next node in tree order, but including nodes in shadow trees,
    /// and only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static Node *NextShadowIncluding(const Node &current, const Node *stayWithin) noexcept;

    /// @brief The node before @p current in tree order (preorder, depth-first), or null if it's the root.
    /// @see https://dom.spec.whatwg.org/#concept-tree-order
    KRYS_NODISCARD static Node *Previous(const Node &current) noexcept;

    /// @brief The node before @p current in tree order, or null if that isn't an inclusive descendant of
    /// @p stayWithin.
    /// @see https://dom.spec.whatwg.org/#concept-tree-order
    KRYS_NODISCARD static Node *Previous(const Node &current, const Node *stayWithin) noexcept;

#pragma endregion

#pragma region Helpers

    /// @see https://dom.spec.whatwg.org/#exclusive-text-node
    KRYS_NODISCARD static bool IsExclusiveTextNode(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#exclusive-text-node
    KRYS_NODISCARD static bool IsExclusiveTextNode(const Node *node) noexcept;

    /// @brief The number of @p node's children (a container's length, where a character data node's is its
    /// data's).
    KRYS_NODISCARD static size_t CountChildNodes(const Node &node) noexcept;

#pragma endregion
  };
}
