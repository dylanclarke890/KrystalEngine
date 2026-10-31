#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::html
{
  class HTMLElement;
}

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
  class Node;
  class ShadowRoot;
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

    /// @return @p element's shadow root, open or closed, or null if @p element isn't a shadow host.
    KRYS_NODISCARD static ShadowRoot *GetShadowRoot(const Element &element) noexcept;

    /// @return The shadow root whose shadow tree contains @p node, or null if @p node isn't in a shadow tree.
    KRYS_NODISCARD static ShadowRoot *ContainingShadowRoot(const Node &node) noexcept;

    /// @return The host of the shadow tree that contains @p node, or null if @p node isn't in a shadow tree.
    KRYS_NODISCARD static Element *ShadowHost(const Node &node) noexcept;

    /// @return @p node's parent element or, if @p node is the child of a shadow root, its host.
    KRYS_NODISCARD static Element *ParentOrShadowHostElement(const Node &node) noexcept;

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

    /// @brief Helper function for getting the next node in tree order, but skipping over the children of the
    /// current node.
    KRYS_NODISCARD static Node *NextSkippingChildren(const Node &current) noexcept;

    /// @brief Helper function for getting the next node in tree order, but skipping over the children of the
    /// current node, and only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static Node *NextSkippingChildren(const Node &current, const Node *stayWithin) noexcept;

    /// @brief Helper function for getting the next node in tree order, but including nodes in shadow trees.
    KRYS_NODISCARD static Node *NextShadowIncluding(const Node &current) noexcept;

    /// @brief Helper function for getting the next node in tree order, but including nodes in shadow trees,
    /// and only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static Node *NextShadowIncluding(const Node &current, const Node *stayWithin) noexcept;

    /// @brief Helper function for getting the next ancestor sibling of a node.
    KRYS_NODISCARD static Node *NextAncestorSibling(const Node &current) noexcept;

    /// @brief Helper function for getting the next ancestor sibling of a node, but only if it is a descendant
    /// of `stayWithin`.
    KRYS_NODISCARD static Node *NextAncestorSibling(const Node &current, const Node *stayWithin) noexcept;

    /// @brief Helper function for getting the next node in tree order that is an 'exclusive' Text node
    /// (non-CDATASection Text node).
    KRYS_NODISCARD static const Text *NextExclusiveTextNode(const Node &current) noexcept;

    /// @brief Helper function for getting the next node in tree order that is an 'exclusive' Text node
    /// (non-CDATASection Text node).
    KRYS_NODISCARD static Text *NextExclusiveTextNode(Node &current) noexcept;

    /// @brief Helper function for getting the next node in tree order that is an 'exclusive' Text node
    /// (non-CDATASection Text node), but only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static Text *NextExclusiveTextNode(const Node &current, const Node *stayWithin) noexcept;

    /// @brief The node before @p current in tree order (preorder, depth-first), or null if it's the root.
    /// @see https://dom.spec.whatwg.org/#concept-tree-order
    KRYS_NODISCARD static Node *Previous(const Node &current) noexcept;

    /// @brief The node before @p current in tree order, or null if that isn't an inclusive descendant of
    /// @p stayWithin.
    /// @see https://dom.spec.whatwg.org/#concept-tree-order
    KRYS_NODISCARD static Node *Previous(const Node &current, const Node *stayWithin) noexcept;

    /// @brief Helper function for getting the previous node in tree order, but skipping over the children of
    /// the current node.
    KRYS_NODISCARD static Node *PreviousSkippingChildren(const Node &current) noexcept;

    /// @brief Helper function for getting the previous node in tree order, but skipping over the children of
    /// the current node, and only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static Node *PreviousSkippingChildren(const Node &current,
                                                         const Node *stayWithin) noexcept;

    /// @brief The previous sibling of @p node, counting an element's `::before` as its first child and its
    /// `::after` as its last.
    KRYS_NODISCARD static Node *PseudoAwarePreviousSibling(const Node &node) noexcept;

    /// @brief The next sibling of @p node, counting an element's `::before` as its first child and its
    /// `::after` as its last.
    KRYS_NODISCARD static Node *PseudoAwareNextSibling(const Node &node) noexcept;

    /// @brief The first child of @p node, counting an element's `::before` as its first child.
    KRYS_NODISCARD static Node *PseudoAwareFirstChild(const Node &node) noexcept;

    /// @brief The last child of @p node, counting an element's `::after` as its last child.
    KRYS_NODISCARD static Node *PseudoAwareLastChild(const Node &node) noexcept;

    /// @brief The previous node in tree order, including `::before` and `::after`, within @p stayWithin.
    KRYS_NODISCARD static Node *PreviousIncludingPseudo(const Node &current,
                                                        const Node *stayWithin = nullptr) noexcept;

    /// @brief The next node in tree order, including `::before` and `::after`, within @p stayWithin.
    KRYS_NODISCARD static Node *NextIncludingPseudo(const Node &current,
                                                    const Node *stayWithin = nullptr) noexcept;

    /// @brief The next node in tree order after @p current's descendants, including `::before` and `::after`,
    /// within @p stayWithin.
    KRYS_NODISCARD static Node *
      NextIncludingPseudoSkippingChildren(const Node &current, const Node *stayWithin = nullptr) noexcept;

    /// @brief The previous element in tree order, including `::before` and `::after`, within @p stayWithin.
    KRYS_NODISCARD static Element *PreviousElementIncludingPseudo(const Node &current,
                                                                  const Node *stayWithin = nullptr) noexcept;

    /// @brief The next element in tree order, including `::before` and `::after`, within @p stayWithin.
    KRYS_NODISCARD static Element *NextElementIncludingPseudo(const Node &current,
                                                              const Node *stayWithin = nullptr) noexcept;

    /// @brief The next element in tree order after @p current's descendants, including `::before` and
    /// `::after`, within @p stayWithin.
    KRYS_NODISCARD static Element *
      NextElementIncludingPseudoSkippingChildren(const Node &current,
                                                 const Node *stayWithin = nullptr) noexcept;

    /// @brief The previous element sibling of @p node, counting `::before` and `::after` as children.
    KRYS_NODISCARD static Element *PseudoAwarePreviousElementSibling(const Node &node) noexcept;

    /// @brief Helper function for getting the last descendant of a node in tree order.
    KRYS_NODISCARD static Node *Last(const ContainerNode &current) noexcept;

    /// @brief Helper function for getting the deepest last child of a node in tree order.
    KRYS_NODISCARD static Node *DeepLastChild(Node &current) noexcept;

    /// @brief Helper function for getting the next sibling of a node that is an Element, or nullptr if there
    /// are no more element siblings.
    KRYS_NODISCARD static const Element *NextElementSibling(const Node &current) noexcept;

    /// @brief Helper function for getting the next sibling of a node that is an Element, or nullptr if there
    /// are no more element siblings.
    KRYS_NODISCARD static Element *NextElementSibling(Node &current) noexcept;

    /// @brief Helper function for getting the previous sibling of a node that is an Element, or nullptr if
    /// there are no more element siblings.
    KRYS_NODISCARD static const Element *PreviousElementSibling(const Node &current) noexcept;

    /// @brief Helper function for getting the previous sibling of a node that is an Element, or nullptr if
    /// there are no more element siblings.
    KRYS_NODISCARD static Element *PreviousElementSibling(Node &current) noexcept;

    /// @brief Helper function for getting the first child of a node that is an Element, or nullptr if there
    /// are no element children.
    KRYS_NODISCARD static const Element *FirstElementChild(const ContainerNode &node) noexcept;

    /// @brief Helper function for getting the first child of a node that is an Element, or nullptr if there
    /// are no element children.
    KRYS_NODISCARD static Element *FirstElementChild(ContainerNode &node) noexcept;

    /// @brief Helper function for getting the last child of a node that is an Element, or nullptr if there
    /// are no element children.
    KRYS_NODISCARD static const Element *LastElementChild(const ContainerNode &node) noexcept;

    /// @brief Helper function for getting the last child of a node that is an Element, or nullptr if there
    /// are no element children.
    KRYS_NODISCARD static Element *LastElementChild(ContainerNode &node) noexcept;

    /// @brief Helper function for getting the next sibling of a node that is an HTMLElement, or nullptr if
    /// there are no more HTMLElement siblings.
    KRYS_NODISCARD static const html::HTMLElement *NextHTMLElementSibling(const Node &current) noexcept;

    /// @brief Helper function for getting the next sibling of a node that is an HTMLElement, or nullptr if
    /// there are no more HTMLElement siblings.
    KRYS_NODISCARD static html::HTMLElement *NextHTMLElementSibling(Node &current) noexcept;

    /// @brief Helper function for getting the previous sibling of a node that is an HTMLElement, or nullptr
    /// if there are no more HTMLElement siblings.
    KRYS_NODISCARD static const html::HTMLElement *PreviousHTMLElementSibling(const Node &current) noexcept;

    /// @brief Helper function for getting the previous sibling of a node that is an HTMLElement, or nullptr
    /// if there are no more HTMLElement siblings.
    KRYS_NODISCARD static html::HTMLElement *PreviousHTMLElementSibling(Node &current) noexcept;

    /// @brief Helper function for getting the first child of a node that is an HTMLElement, or nullptr if
    /// there are no HTMLElement children.
    KRYS_NODISCARD static const html::HTMLElement *FirstHTMLElementChild(const ContainerNode &node) noexcept;

    /// @brief Helper function for getting the first child of a node that is an HTMLElement, or nullptr if
    /// there are no HTMLElement children.
    KRYS_NODISCARD static html::HTMLElement *FirstHTMLElementChild(ContainerNode &node) noexcept;

    /// @brief Helper function for getting the last child of a node that is an HTMLElement, or nullptr if
    /// there are no HTMLElement children.
    KRYS_NODISCARD static const html::HTMLElement *LastHTMLElementChild(const ContainerNode &node) noexcept;

    /// @brief Helper function for getting the last child of a node that is an HTMLElement, or nullptr if
    /// there are no HTMLElement children.
    KRYS_NODISCARD static html::HTMLElement *LastHTMLElementChild(ContainerNode &node) noexcept;

#pragma endregion

#pragma region Post Order Traversal

    /// @brief Helper function for getting the next node in post-order tree order.
    KRYS_NODISCARD static Node *NextPostOrder(const Node &current) noexcept;

    /// @brief Helper function for getting the next node in post-order tree order, but only if it is a
    /// descendant of `stayWithin`.
    KRYS_NODISCARD static Node *NextPostOrder(const Node &current, const Node *stayWithin) noexcept;

    /// @brief Helper function for getting the previous node in post-order tree order.
    KRYS_NODISCARD static Node *PreviousPostOrder(const Node &current) noexcept;

    /// @brief Helper function for getting the previous node in post-order tree order, but only if it is a
    /// descendant of `stayWithin`.
    KRYS_NODISCARD static Node *PreviousPostOrder(const Node &current, const Node *stayWithin) noexcept;

    /// @brief Helper function for getting the previous node in post-order tree order, but skipping over the
    /// children of the current node.
    KRYS_NODISCARD static Node *PreviousSkippingChildrenPostOrder(const Node &current) noexcept;

    /// @brief Helper function for getting the previous node in post-order tree order, but skipping over the
    /// children of the current node, and only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static Node *PreviousSkippingChildrenPostOrder(const Node &current,
                                                                  const Node *stayWithin) noexcept;

    /// @brief Helper function for getting the previous ancestor sibling of a node in post-order tree order.
    KRYS_NODISCARD static Node *PreviousAncestorSiblingPostOrder(const Node &current) noexcept;

    /// @brief Helper function for getting the previous ancestor sibling of a node in post-order tree order,
    /// but only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static Node *PreviousAncestorSiblingPostOrder(const Node &current,
                                                                 const Node *stayWithin) noexcept;

#pragma endregion

#pragma region Helpers

    /// @brief Helper function for checking that `a` and `b` share the same root.
    /// @note This does not mean that they are in a document/shadow tree.
    KRYS_NODISCARD static bool SameRoot(const Node &a, const Node &b) noexcept;

    /// @brief Helper function for getting the closest common ancestor of two nodes.
    KRYS_NODISCARD static ContainerNode *CommonAncestor(Node &a, Node &b) noexcept;

    /// @brief Helper function for getting the closest common ancestor of two nodes.
    KRYS_NODISCARD static const ContainerNode *CommonAncestor(const Node &a, const Node &b) noexcept;

    /// @brief Helper function for getting the nth child of a node.
    KRYS_NODISCARD static const Node *ChildAt(const ContainerNode &node, size_t index) noexcept;

    /// @brief Helper function for getting the nth child of a node.
    KRYS_NODISCARD static Node *ChildAt(ContainerNode &node, size_t index) noexcept;

    /// @see https://dom.spec.whatwg.org/#exclusive-text-node
    KRYS_NODISCARD static bool IsExclusiveTextNode(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#exclusive-text-node
    KRYS_NODISCARD static bool IsExclusiveTextNode(const Node *node) noexcept;

#pragma endregion
  };
}