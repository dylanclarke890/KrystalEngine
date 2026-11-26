#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::web
{
  class ContainerNode;
  class Element;
  class HTMLElement;
  class Node;
  class ShadowRoot;
  class Text;
}

namespace krys::web::dom
{
  /// @brief Tree helpers the specs don't define: traversal by pseudo-element, element and post order,
  /// shadow hosts and roots, common ancestors and the like. The DOM's own tree concepts are in TreeQueries.
  class TreeTraversal
  {
  public:
#pragma region Shadow Roots

    /// @return @p element's shadow root, open or closed, or null if @p element isn't a shadow host.
    KRYS_NODISCARD static ShadowRoot *GetShadowRoot(const Element &element) noexcept;

    /// @return The shadow root whose shadow tree contains @p node, or null if @p node isn't in a shadow tree.
    KRYS_NODISCARD static ShadowRoot *ContainingShadowRoot(const Node &node) noexcept;

    /// @return The host of the shadow tree that contains @p node, or null if @p node isn't in a shadow tree.
    KRYS_NODISCARD static Element *ShadowHost(const Node &node) noexcept;

    /// @return @p node's parent element or, if @p node is the child of a shadow root, its host.
    KRYS_NODISCARD static Element *ParentOrShadowHostElement(const Node &node) noexcept;

    /// @return @p node's parent or, if @p node is a shadow root, its host.
    KRYS_NODISCARD static ContainerNode *ParentOrShadowHostNode(const Node &node) noexcept;

    /// @return @p node's parent, unless it's a shadow root.
    KRYS_NODISCARD static ContainerNode *NonShadowBoundaryParentNode(const Node &node) noexcept;

    /// @return The inclusive ancestor of @p node that's a child of a shadow root, or null outside shadow
    /// trees. @p node mustn't be a shadow root.
    KRYS_NODISCARD static Node *NonBoundaryShadowTreeRootNode(Node &node) noexcept;

#pragma endregion

#pragma region Tree Order Traversal

    /// @brief Helper function for getting the next node in tree order, but skipping over the children of the
    /// current node.
    KRYS_NODISCARD static Node *NextSkippingChildren(const Node &current) noexcept;

    /// @brief Helper function for getting the next node in tree order, but skipping over the children of the
    /// current node, and only if it is a descendant of `stayWithin`.
    KRYS_NODISCARD static Node *NextSkippingChildren(const Node &current, const Node *stayWithin) noexcept;

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

    /// @brief Helper function for getting the deepest last child of a node in tree order.
    KRYS_NODISCARD static const Node *DeepLastChild(const Node &current) noexcept;

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
    KRYS_NODISCARD static const HTMLElement *NextHTMLElementSibling(const Node &current) noexcept;

    /// @brief Helper function for getting the next sibling of a node that is an HTMLElement, or nullptr if
    /// there are no more HTMLElement siblings.
    KRYS_NODISCARD static HTMLElement *NextHTMLElementSibling(Node &current) noexcept;

    /// @brief Helper function for getting the previous sibling of a node that is an HTMLElement, or nullptr
    /// if there are no more HTMLElement siblings.
    KRYS_NODISCARD static const HTMLElement *PreviousHTMLElementSibling(const Node &current) noexcept;

    /// @brief Helper function for getting the previous sibling of a node that is an HTMLElement, or nullptr
    /// if there are no more HTMLElement siblings.
    KRYS_NODISCARD static HTMLElement *PreviousHTMLElementSibling(Node &current) noexcept;

    /// @brief Helper function for getting the first child of a node that is an HTMLElement, or nullptr if
    /// there are no HTMLElement children.
    KRYS_NODISCARD static const HTMLElement *FirstHTMLElementChild(const ContainerNode &node) noexcept;

    /// @brief Helper function for getting the first child of a node that is an HTMLElement, or nullptr if
    /// there are no HTMLElement children.
    KRYS_NODISCARD static HTMLElement *FirstHTMLElementChild(ContainerNode &node) noexcept;

    /// @brief Helper function for getting the last child of a node that is an HTMLElement, or nullptr if
    /// there are no HTMLElement children.
    KRYS_NODISCARD static const HTMLElement *LastHTMLElementChild(const ContainerNode &node) noexcept;

    /// @brief Helper function for getting the last child of a node that is an HTMLElement, or nullptr if
    /// there are no HTMLElement children.
    KRYS_NODISCARD static HTMLElement *LastHTMLElementChild(ContainerNode &node) noexcept;

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

    /// @brief The child of @p node at @p index, or null if it has fewer children (or can't have any).
    KRYS_NODISCARD static const Node *ChildAt(const Node &node, size_t index) noexcept;

    /// @brief The child of @p node at @p index, or null if it has fewer children (or can't have any).
    KRYS_NODISCARD static Node *ChildAt(Node &node, size_t index) noexcept;

#pragma endregion
  };
}
