#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Document;
  class Element;
  class Node;

  /// @brief Implementations of the various tree concepts/algorithms defined in the DOM spec, grouped together
  /// for ease of use. Also contains additional helper functions related to tree queries that aren't
  /// explicitly defined in the spec, but are useful for implementing the algorithms.
  /// @see https://dom.spec.whatwg.org/#trees
  /// @see https://dom.spec.whatwg.org/#node-trees
  /// @see https://dom.spec.whatwg.org/#document-trees
  /// @see https://dom.spec.whatwg.org/#shadow-trees
  /// @see https://dom.spec.whatwg.org/#interface-documentfragment
  class TreeQueries
  {
  public:
#pragma region Trees - https://dom.spec.whatwg.org/#trees

    /// @see https://dom.spec.whatwg.org/#concept-tree-participate
    KRYS_NODISCARD static bool IsParent(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-participate
    KRYS_NODISCARD static bool IsChild(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-root
    KRYS_NODISCARD static const Node &Root(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-root
    KRYS_NODISCARD static Node &Root(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-descendant
    KRYS_NODISCARD static bool IsDescendant(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-inclusive-descendant
    KRYS_NODISCARD static bool IsInclusiveDescendant(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-ancestor
    KRYS_NODISCARD static bool IsAncestor(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-inclusive-ancestor
    KRYS_NODISCARD static bool IsInclusiveAncestor(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-sibling
    KRYS_NODISCARD static bool IsSibling(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-inclusive-sibling
    KRYS_NODISCARD static bool IsInclusiveSibling(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-preceding
    KRYS_NODISCARD static bool IsPreceding(const Node &a, const Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-following
    KRYS_NODISCARD static bool IsFollowing(const Node &a, const Node &b) noexcept;

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
    KRYS_NODISCARD static RawPtr<const Element> DocumentElement(const Document &document) noexcept;

    /// @see https://dom.spec.whatwg.org/#document-element
    KRYS_NODISCARD static RawPtr<Element> DocumentElement(Document &document) noexcept;

#pragma endregion

#pragma region Shadow Trees - https://dom.spec.whatwg.org/#shadow-trees

    /// @see https://dom.spec.whatwg.org/#concept-shadow-tree
    KRYS_NODISCARD static bool IsInShadowTree(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-tree-host-including-inclusive-ancestor
    KRYS_NODISCARD static bool IsHostIncludingInclusiveAncestorOf(const Node &a, const Node &b) noexcept;

#pragma endregion

#pragma region Helpers

    /// @brief Helper function for checking that `a` and `b` share the same root.
    /// @note This does not mean that they are in a document/shadow tree.
    KRYS_NODISCARD static bool SameRoot(const Node &a, const Node &b) noexcept;

    /// @brief Helper function for getting the closest common ancestor of two nodes.
    KRYS_NODISCARD static RawPtr<ContainerNode> CommonAncestor(Node &a, Node &b) noexcept;

    /// @brief Helper function for getting the closest common ancestor of two nodes.
    KRYS_NODISCARD static RawPtr<const ContainerNode> CommonAncestor(const Node &a, const Node &b) noexcept;

    /// @brief Helper function for getting the nth child of a node.
    KRYS_NODISCARD static RawPtr<const Node> ChildAt(const ContainerNode &node, size_t index) noexcept;

    /// @brief Helper function for getting the nth child of a node.
    KRYS_NODISCARD static RawPtr<Node> ChildAt(ContainerNode &node, size_t index) noexcept;

#pragma endregion
  };
}