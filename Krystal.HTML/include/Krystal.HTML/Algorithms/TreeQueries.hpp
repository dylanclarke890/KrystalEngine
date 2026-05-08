#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Utils/SmallNodeList.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Element;
  class EventTarget;
  class HTMLSlotElement;
  class Node;
  class Text;

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
    KRYS_NODISCARD static size_t Length(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-empty
    KRYS_NODISCARD static bool IsEmpty(Node &node) noexcept;

#pragma endregion

#pragma region Document Trees - https://dom.spec.whatwg.org/#document-trees

    /// @see https://dom.spec.whatwg.org/#in-a-document-tree
    KRYS_NODISCARD static bool IsInDocumentTree(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#document-element
    KRYS_NODISCARD static RawPtr<const Element> DocumentElement(const Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#document-element
    KRYS_NODISCARD static RawPtr<Element> DocumentElement(Node &node) noexcept;

#pragma endregion

#pragma region Shadow Trees - https://dom.spec.whatwg.org/#shadow-trees

    KRYS_NODISCARD static bool IsInShadowTree(const Node &node) noexcept;

#pragma endregion

    /// @see https://dom.spec.whatwg.org/#concept-tree-host-including-inclusive-ancestor
    KRYS_NODISCARD static bool IsHostIncludingInclusiveAncestorOf(Node &a, Node &b) noexcept;

    /// @see https://dom.spec.whatwg.org/#locate-a-namespace-prefix
    KRYS_NODISCARD static DOMStringAtom LocateNamespacePrefix(const Element &element,
                                                              DOMStringAtom namespaceURI) noexcept;

    /// @see https://dom.spec.whatwg.org/#locate-a-namespace
    KRYS_NODISCARD static DOMStringAtom LocateNamespace(const Node &node, DOMStringAtom prefix) noexcept;

    KRYS_NODISCARD static bool HasSameRoot(const Node &a, const Node &b) noexcept;

    KRYS_NODISCARD static bool IsConnectedInSameTreeScope(const Node &a, const Node &b) noexcept;
    KRYS_NODISCARD static bool IsDocTypeOrDocTypeFollows(RawPtr<Node> node) noexcept;
    KRYS_NODISCARD static bool IsExclusiveTextNode(const Node &node) noexcept;

    KRYS_NODISCARD static RawPtr<const Node> ChildAt(const ContainerNode &node, size_t index) noexcept;
    KRYS_NODISCARD static RawPtr<Node> ChildAt(ContainerNode &node, size_t index) noexcept;

    KRYS_NODISCARD static size_t ChildNodeCount(const ContainerNode &node) noexcept;
    KRYS_NODISCARD static size_t ChildElementCount(const ContainerNode &node) noexcept;
    KRYS_NODISCARD static bool HasElementChild(const ContainerNode &node) noexcept;

    static void CollectChildNodes(ContainerNode &parent, SmallNodeList &collection) noexcept;
    static void CollectChildElements(ContainerNode &parent, SmallElementList &collection) noexcept;

    KRYS_NODISCARD static List<Ref<Node>> InclusiveAncestors(Node &node) noexcept;

    KRYS_NODISCARD static DOMString DescendantTextContent(const ContainerNode &node) noexcept;
    KRYS_NODISCARD static DOMString ChildTextContent(const ContainerNode &node) noexcept;
    KRYS_NODISCARD static DOMString ContiguousTextContent(const Text &node) noexcept;
    KRYS_NODISCARD static DOMString ContiguousExclusiveTextContent(const Text &node) noexcept;
    KRYS_NODISCARD static DOMString FollowingContiguousExclusiveTextContent(const Text &node) noexcept;

    KRYS_NODISCARD static RawPtr<ContainerNode> CommonAncestorContainer(Node &a, Node &b) noexcept;
  };
}