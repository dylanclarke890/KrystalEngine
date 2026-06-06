#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Document;
  class Element;
  class HTMLElement;
  class Node;
  class Text;

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

#pragma region Shadow Roots

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-root
    KRYS_NODISCARD static const Node &ShadowIncludingRoot(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-root
    KRYS_NODISCARD static Node &ShadowIncludingRoot(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-shadow-including-descendant
    KRYS_NODISCARD static bool IsShadowIncludingDescendant(const Node &a, const Node &b) noexcept;

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

    /// @brief Helper function for getting the next node in tree order.
    KRYS_NODISCARD static RawPtr<Node> Next(const Node &current) noexcept;

    /// @brief Helper function for getting the next node in tree order.
    KRYS_NODISCARD static RawPtr<Node> Next(const ContainerNode &current) noexcept;

    /// @brief Helper function for getting the next node in tree order, but only if it is a descendant of
    /// `stayWithin`.
    KRYS_NODISCARD static RawPtr<Node> Next(const Node &current, RawPtr<const Node> stayWithin) noexcept;

    /// @brief Helper function for getting the next node in tree order, but only if it is a descendant of
    /// `stayWithin`.
    KRYS_NODISCARD static RawPtr<Node> Next(const ContainerNode &current,
                                            RawPtr<const Node> stayWithin) noexcept;

    /// @brief Helper function for getting the next node in tree order, but skipping over the children of the
    /// current node.
    KRYS_NODISCARD static RawPtr<Node> NextSkippingChildren(const Node &current) noexcept;

    /// @brief Helper function for getting the next node in tree order, but skipping over the children of the
    /// current node, and only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static RawPtr<Node> NextSkippingChildren(const Node &current,
                                                            RawPtr<const Node> stayWithin) noexcept;

    /// @brief Helper function for getting the next node in tree order, but including nodes in shadow trees.
    KRYS_NODISCARD static RawPtr<Node> NextShadowIncluding(const Node &current) noexcept;

    /// @brief Helper function for getting the next node in tree order, but including nodes in shadow trees,
    /// and only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static RawPtr<Node> NextShadowIncluding(const Node &current,
                                                           RawPtr<const Node> stayWithin) noexcept;

    /// @brief Helper function for getting the next ancestor sibling of a node.
    KRYS_NODISCARD static RawPtr<Node> NextAncestorSibling(const Node &current) noexcept;

    /// @brief Helper function for getting the next ancestor sibling of a node, but only if it is a descendant
    /// of `stayWithin`.
    KRYS_NODISCARD static RawPtr<Node> NextAncestorSibling(const Node &current,
                                                           RawPtr<const Node> stayWithin) noexcept;

    /// @brief Helper function for getting the next node in tree order that is an 'exclusive' Text node
    /// (non-CDATASection Text node).
    KRYS_NODISCARD static RawPtr<const Text> NextExclusiveTextNode(const Node &current) noexcept;

    /// @brief Helper function for getting the next node in tree order that is an 'exclusive' Text node
    /// (non-CDATASection Text node).
    KRYS_NODISCARD static RawPtr<Text> NextExclusiveTextNode(Node &current) noexcept;

    /// @brief Helper function for getting the next node in tree order that is an 'exclusive' Text node
    /// (non-CDATASection Text node), but only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static RawPtr<Text> NextExclusiveTextNode(const Node &current,
                                                             RawPtr<const Node> stayWithin) noexcept;

    /// @brief Helper function for getting the previous node in tree order.
    KRYS_NODISCARD static RawPtr<Node> Previous(const Node &current) noexcept;

    /// @brief Helper function for getting the previous node in tree order, but only if it is a descendant of
    /// `stayWithin`.
    KRYS_NODISCARD static RawPtr<Node> Previous(const Node &current, RawPtr<const Node> stayWithin) noexcept;

    /// @brief Helper function for getting the previous node in tree order, but skipping over the children of
    /// the current node.
    KRYS_NODISCARD static RawPtr<Node> PreviousSkippingChildren(const Node &current) noexcept;

    /// @brief Helper function for getting the previous node in tree order, but skipping over the children of
    /// the current node, and only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static RawPtr<Node> PreviousSkippingChildren(const Node &current,
                                                                RawPtr<const Node> stayWithin) noexcept;

    /// @brief Helper function for getting the last descendant of a node in tree order.
    KRYS_NODISCARD static RawPtr<Node> Last(const ContainerNode &current) noexcept;

    /// @brief Helper function for getting the deepest last child of a node in tree order.
    KRYS_NODISCARD static RawPtr<Node> DeepLastChild(Node &current) noexcept;

    /// @brief Helper function for getting the next sibling of a node that is an Element, or nullptr if there
    /// are no more element siblings.
    KRYS_NODISCARD static RawPtr<const Element> NextElementSibling(const Node &current) noexcept;

    /// @brief Helper function for getting the next sibling of a node that is an Element, or nullptr if there
    /// are no more element siblings.
    KRYS_NODISCARD static RawPtr<Element> NextElementSibling(Node &current) noexcept;

    /// @brief Helper function for getting the previous sibling of a node that is an Element, or nullptr if
    /// there are no more element siblings.
    KRYS_NODISCARD static RawPtr<const Element> PreviousElementSibling(const Node &current) noexcept;

    /// @brief Helper function for getting the previous sibling of a node that is an Element, or nullptr if
    /// there are no more element siblings.
    KRYS_NODISCARD static RawPtr<Element> PreviousElementSibling(Node &current) noexcept;

    /// @brief Helper function for getting the first child of a node that is an Element, or nullptr if there
    /// are no element children.
    KRYS_NODISCARD static RawPtr<const Element> FirstElementChild(const ContainerNode &node) noexcept;

    /// @brief Helper function for getting the first child of a node that is an Element, or nullptr if there
    /// are no element children.
    KRYS_NODISCARD static RawPtr<Element> FirstElementChild(ContainerNode &node) noexcept;

    /// @brief Helper function for getting the last child of a node that is an Element, or nullptr if there
    /// are no element children.
    KRYS_NODISCARD static RawPtr<const Element> LastElementChild(const ContainerNode &node) noexcept;

    /// @brief Helper function for getting the last child of a node that is an Element, or nullptr if there
    /// are no element children.
    KRYS_NODISCARD static RawPtr<Element> LastElementChild(ContainerNode &node) noexcept;

    /// @brief Helper function for getting the next sibling of a node that is an HTMLElement, or nullptr if
    /// there are no more HTMLElement siblings.
    KRYS_NODISCARD static RawPtr<const HTMLElement> NextHTMLElementSibling(const Node &current) noexcept;

    /// @brief Helper function for getting the next sibling of a node that is an HTMLElement, or nullptr if
    /// there are no more HTMLElement siblings.
    KRYS_NODISCARD static RawPtr<HTMLElement> NextHTMLElementSibling(Node &current) noexcept;

    /// @brief Helper function for getting the previous sibling of a node that is an HTMLElement, or nullptr
    /// if there are no more HTMLElement siblings.
    KRYS_NODISCARD static RawPtr<const HTMLElement> PreviousHTMLElementSibling(const Node &current) noexcept;

    /// @brief Helper function for getting the previous sibling of a node that is an HTMLElement, or nullptr
    /// if there are no more HTMLElement siblings.
    KRYS_NODISCARD static RawPtr<HTMLElement> PreviousHTMLElementSibling(Node &current) noexcept;

    /// @brief Helper function for getting the first child of a node that is an HTMLElement, or nullptr if
    /// there are no HTMLElement children.
    KRYS_NODISCARD static RawPtr<const HTMLElement> FirstHTMLElementChild(const ContainerNode &node) noexcept;

    /// @brief Helper function for getting the first child of a node that is an HTMLElement, or nullptr if
    /// there are no HTMLElement children.
    KRYS_NODISCARD static RawPtr<HTMLElement> FirstHTMLElementChild(ContainerNode &node) noexcept;

    /// @brief Helper function for getting the last child of a node that is an HTMLElement, or nullptr if
    /// there are no HTMLElement children.
    KRYS_NODISCARD static RawPtr<const HTMLElement> LastHTMLElementChild(const ContainerNode &node) noexcept;

    /// @brief Helper function for getting the last child of a node that is an HTMLElement, or nullptr if
    /// there are no HTMLElement children.
    KRYS_NODISCARD static RawPtr<HTMLElement> LastHTMLElementChild(ContainerNode &node) noexcept;

#pragma endregion

#pragma region Post Order Traversal

    /// @brief Helper function for getting the next node in post-order tree order.
    KRYS_NODISCARD static RawPtr<Node> NextPostOrder(const Node &current) noexcept;

    /// @brief Helper function for getting the next node in post-order tree order, but only if it is a
    /// descendant of `stayWithin`.
    KRYS_NODISCARD static RawPtr<Node> NextPostOrder(const Node &current,
                                                     RawPtr<const Node> stayWithin) noexcept;

    /// @brief Helper function for getting the previous node in post-order tree order.
    KRYS_NODISCARD static RawPtr<Node> PreviousPostOrder(const Node &current) noexcept;

    /// @brief Helper function for getting the previous node in post-order tree order, but only if it is a
    /// descendant of `stayWithin`.
    KRYS_NODISCARD static RawPtr<Node> PreviousPostOrder(const Node &current,
                                                         RawPtr<const Node> stayWithin) noexcept;

    /// @brief Helper function for getting the previous node in post-order tree order, but skipping over the
    /// children of the current node.
    KRYS_NODISCARD static RawPtr<Node> PreviousSkippingChildrenPostOrder(const Node &current) noexcept;

    /// @brief Helper function for getting the previous node in post-order tree order, but skipping over the
    /// children of the current node, and only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static RawPtr<Node>
      PreviousSkippingChildrenPostOrder(const Node &current, RawPtr<const Node> stayWithin) noexcept;

    /// @brief Helper function for getting the previous ancestor sibling of a node in post-order tree order.
    KRYS_NODISCARD static RawPtr<Node> PreviousAncestorSiblingPostOrder(const Node &current) noexcept;

    /// @brief Helper function for getting the previous ancestor sibling of a node in post-order tree order,
    /// but only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static RawPtr<Node>
      PreviousAncestorSiblingPostOrder(const Node &current, RawPtr<const Node> stayWithin) noexcept;

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

    /// @see https://dom.spec.whatwg.org/#exclusive-text-node
    KRYS_NODISCARD static bool IsExclusiveTextNode(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#exclusive-text-node
    KRYS_NODISCARD static bool IsExclusiveTextNode(RawPtr<const Node> node) noexcept;

#pragma endregion
  };
}