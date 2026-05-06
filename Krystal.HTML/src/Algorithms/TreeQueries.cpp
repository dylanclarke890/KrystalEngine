#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Namespaces.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include <ranges>

namespace Krys::HTML
{
#pragma region Node Trees - https://dom.spec.whatwg.org/#node-trees

  size_t TreeQueries::Length(Node &node) noexcept
  {
    if (node.IsDocumentTypeNode() || node.IsAttributeNode())
    {
      return 0;
    }

    if (auto *characterData = DynamicDowncast<CharacterData>(node))
    {
      return characterData->Data().size();
    }

    return node.CountChildNodes();
  }

  bool TreeQueries::IsEmpty(Node &node) noexcept
  {
    return Length(node) == 0;
  }

#pragma endregion

#pragma region Document Trees

  bool TreeQueries::IsInDocumentTree(const Node &node) noexcept
  {
    return Is<Document>(Root(node));
  }

  RawPtr<const Element> TreeQueries::DocumentElement(const Node &node) noexcept
  {
    if (auto *document = DynamicDowncast<Document>(Root(node)))
    {
      return TreeTraversal::FirstElementChild(*document);
    }

    return nullptr;
  }

  RawPtr<Element> TreeQueries::DocumentElement(Node &node) noexcept
  {
    if (auto *document = DynamicDowncast<Document>(Root(node)))
    {
      return TreeTraversal::FirstElementChild(*document);
    }

    return nullptr;
  }

#pragma endregion

#pragma region Shadow Trees

  bool TreeQueries::IsInShadowTree(const Node &node) noexcept
  {
    return Is<ShadowRoot>(Root(node));
  }

#pragma endregion

#pragma region Trees

  bool TreeQueries::IsParent(const Node &a, const Node &b) noexcept
  {
    return &a == b.ParentNode();
  }

  bool TreeQueries::IsChild(const Node &a, const Node &b) noexcept
  {
    return a.ParentNode() == &b;
  }

  const Node &TreeQueries::Root(const Node &node) noexcept
  {
    if (node.IsInTreeScope())
    {
      return node.GetTreeScope().RootNode();
    }

    return TreeTraversal::Root(node);
  }

  Node &TreeQueries::Root(Node &node) noexcept
  {
    if (node.IsInTreeScope())
    {
      return node.GetTreeScope().RootNode();
    }

    return TreeTraversal::Root(node);
  }

  bool TreeQueries::IsDescendant(const Node &a, const Node &b) noexcept
  {
    return std::ranges::any_of(ConstAncestorRange(a), [&](const Node &n) { return &n == &b; });
  }

  bool TreeQueries::IsInclusiveDescendant(const Node &a, const Node &b) noexcept
  {
    return std::ranges::any_of(ConstInclusiveAncestorRange(a), [&](const Node &n) { return &n == &b; });
  }

  bool TreeQueries::IsAncestor(const Node &a, const Node &b) noexcept
  {
    return IsDescendant(b, a);
  }

  bool TreeQueries::IsInclusiveAncestor(const Node &a, const Node &b) noexcept
  {
    return IsInclusiveDescendant(b, a);
  }

  bool TreeQueries::IsSibling(const Node &a, const Node &b) noexcept
  {
    if (a.ParentNode() == nullptr || b.ParentNode() == nullptr || &a == &b)
    {
      return false;
    }

    return a.ParentNode() == b.ParentNode();
  }

  bool TreeQueries::IsInclusiveSibling(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return true;
    }

    return IsSibling(a, b);
  }

  bool TreeQueries::IsPreceding(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return false;
    }

    return std::ranges::any_of(ConstPrecedingRange(b), [&](const Node &current) { return &current == &a; });
  }

  bool TreeQueries::IsFollowing(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return false;
    }

    return std::ranges::any_of(ConstPrecedingRange(a), [&](const Node &current) { return &current == &b; });
  }

  size_t TreeQueries::Index(const Node &node) noexcept
  {
    return Krys::HTML::Count(ConstPreviousSiblingRange(node));
  }

#pragma endregion

#pragma region ShadowRoot

  const Node &TreeQueries::ShadowIncludingRoot(const Node &node) noexcept
  {
    auto &root = Root(node);
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(root))
    {
      auto *host = shadowRoot->Host();
      return host ? ShadowIncludingRoot(*host) : root;
    }

    return root;
  }

  Node &TreeQueries::ShadowIncludingRoot(Node &node) noexcept
  {
    auto &root = Root(node);
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(root))
    {
      auto *host = shadowRoot->Host();
      return host ? ShadowIncludingRoot(*host) : root;
    }

    return root;
  }

  bool TreeQueries::IsShadowIncludingDescendant(const Node &a, const Node &b) noexcept
  {
    if (IsDescendant(a, b))
    {
      return true;
    }

    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(Root(a)))
    {
      if (auto *host = shadowRoot->Host())
      {
        return IsShadowIncludingDescendant(*host, b);
      }
    }

    return false;
  }

  bool TreeQueries::IsShadowIncludingInclusiveDescendant(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return true;
    }

    return IsShadowIncludingDescendant(a, b);
  }

  bool TreeQueries::IsShadowIncludingAncestor(const Node &a, const Node &b) noexcept
  {
    return IsShadowIncludingDescendant(b, a);
  }

  bool TreeQueries::IsShadowIncludingInclusiveAncestor(const Node &a, const Node &b) noexcept
  {
    return IsShadowIncludingInclusiveDescendant(b, a);
  }

  bool TreeQueries::IsClosedShadowHidden(const Node &a, const Node &b) noexcept
  {
    auto &aRoot = Root(a);
    auto *aShadowRoot = DynamicDowncast<ShadowRoot>(aRoot);
    if (aShadowRoot == nullptr)
    {
      return false;
    }

    if (IsShadowIncludingInclusiveAncestor(aRoot, b))
    {
      return false;
    }

    if (aShadowRoot->Mode() == ShadowRootMode::Closed)
    {
      return true;
    }

    if (aShadowRoot->Host() && IsClosedShadowHidden(*aShadowRoot->Host(), b))
    {
      return true;
    }

    return false;
  }

  RawPtr<EventTarget> TreeQueries::Retarget(RawPtr<EventTarget> a, EventTarget &b) noexcept
  {
    auto *current = a;
    auto *bNode = DynamicDowncast<Node>(b);
    while (true)
    {
      if (current == nullptr)
      {
        return nullptr;
      }

      if (!current->IsNode())
      {
        return current;
      }

      auto *currentNode = Downcast<Node>(current);
      auto &currentRoot = Root(*currentNode);
      if (!currentNode->IsShadowRootNode())
      {
        return current;
      }

      if (bNode && !IsShadowIncludingInclusiveAncestor(currentRoot, *bNode))
      {
        return current;
      }

      auto *host = Downcast<ShadowRoot>(&currentRoot)->Host();
      current = host;
    }

    // The spec handles the necessary cases to ensure we never get here, but we need this to satisfy the
    // compiler that current is always valid.
    std::unreachable();
    return current;
  }

#pragma endregion

  bool TreeQueries::IsHostIncludingInclusiveAncestorOf(Node &a, Node &b) noexcept
  {
    if (IsInclusiveAncestor(a, b))
    {
      return true;
    }

    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(Root(b)))
    {
      if (auto *host = shadowRoot->Host())
      {
        return IsHostIncludingInclusiveAncestorOf(a, *host);
      }
    }

    return false;
  }

  bool TreeQueries::IsShadowHost(const Node &node) noexcept
  {
    if (!node.IsElementNode())
    {
      return false;
    }

    return IsShadowHost(Downcast<Element>(node));
  }

  bool TreeQueries::IsShadowHost(const Element &node) noexcept
  {
    return node.ShadowRoot() != nullptr;
  }

  DOMStringAtom TreeQueries::LocateNamespacePrefix(const Element &element,
                                                   DOMStringAtom namespaceURI) noexcept
  {
    if (element.NamespaceURI() == namespaceURI && element.Prefix() != DOMStringAtom::Null())
    {
      return element.Prefix();
    }

    for (auto &attribute : element._attributes)
    {
      if (attribute->Prefix() == u8"xmlns" && attribute->Value() == namespaceURI)
      {
        return attribute->LocalName();
      }
    }

    RawPtr<const Element> parentElement = element.ParentElement();
    if (parentElement != nullptr)
    {
      return LocateNamespacePrefix(*parentElement, namespaceURI);
    }

    return DOMStringAtom::Null();
  }

  DOMStringAtom TreeQueries::LocateNamespace(const Node &node, DOMStringAtom prefix) noexcept
  {
    switch (node.NodeType())
    {
      case NodeType::ELEMENT_NODE:
      {
        if (prefix == Namespaces::XML)
        {
          return Namespaces::XML;
        }

        if (prefix == Namespaces::XMLNSPrefix)
        {
          return Namespaces::XMLNS;
        }

        auto &element = Downcast<Element>(node);
        if (element.NamespaceURI() != DOMStringAtom::Null() && element.Prefix() == prefix)
        {
          return element.NamespaceURI();
        }

        for (auto &attribute : element._attributes)
        {
          if (attribute->NamespaceURI() == Namespaces::XMLNS && attribute->Prefix() == Namespaces::XMLNSPrefix
              && attribute->LocalName() == prefix)
          {
            return attribute->Value().empty() ? DOMStringAtom::Null() : attribute->Value();
          }

          if (prefix == DOMStringAtom::Null() && attribute->NamespaceURI() == Namespaces::XMLNS
              && attribute->Prefix() == DOMStringAtom::Null()
              && attribute->LocalName() == Namespaces::XMLNSPrefix)
          {
            return attribute->Value().empty() ? DOMStringAtom::Null() : attribute->Value();
          }
        }

        RawPtr<const Element> parentElement = element.ParentElement();
        if (parentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return LocateNamespace(*parentElement, prefix);
      }
      case NodeType::DOCUMENT_NODE:
      {
        RawPtr<const Element> documentElement = DocumentElement(node);
        if (documentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return LocateNamespace(*documentElement, prefix);
      }
      case NodeType::DOCUMENT_TYPE_NODE:
      case NodeType::DOCUMENT_FRAGMENT_NODE:
      {
        return DOMStringAtom::Null();
      }
      case NodeType::ATTRIBUTE_NODE:
      {
        RawPtr<const Element> ownerElement = Downcast<Attr>(node).OwnerElement();
        if (ownerElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return LocateNamespace(*ownerElement, prefix);
      }
      default:
      {
        RawPtr<const Element> parentElement = node.ParentElement();
        if (parentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return LocateNamespace(*parentElement, prefix);
      }
    }
  }

  bool TreeQueries::HasSameRoot(const Node &a, const Node &b) noexcept
  {
    return &Root(a) == &Root(b);
  }

  bool TreeQueries::HasSameShadowIncludingRoot(const Node &a, const Node &b) noexcept
  {
    return &ShadowIncludingRoot(a) == &ShadowIncludingRoot(b);
  }

  bool TreeQueries::IsConnectedInSameTreeScope(const Node &a, const Node &b) noexcept
  {
    return a.IsConnected() == b.IsConnected() && &a.GetTreeScope() == &b.GetTreeScope();
  }

  bool TreeQueries::IsDocTypeOrDocTypeFollows(RawPtr<Node> node) noexcept
  {
    if (node == nullptr)
    {
      return false;
    }

    if (node->IsDocumentTypeNode())
    {
      return true;
    }

    return std::ranges::any_of(ConstFollowingRange(*node), [](auto &c) { return c.IsDocumentTypeNode(); });
  }

  bool TreeQueries::IsExclusiveTextNode(const Node &node) noexcept
  {
    return node.IsTextNode() && !node.IsCDATASectionNode();
  }

  RefPtr<ShadowRoot> TreeQueries::GetShadowRoot(const Node &node) noexcept
  {
    if (auto *element = DynamicDowncast<Element>(node))
    {
      return element->ShadowRoot();
    }

    return nullptr;
  }

  RawPtr<Node> TreeQueries::ChildAt(ContainerNode &node, size_t index) noexcept
  {
    size_t i = 0;
    auto children = ChildNodeRange(node);
    auto it = std::ranges::find_if(children, [&](const Node &) { return i++ == index; });
    return it == std::ranges::end(children) ? nullptr : &*it;
  }

  size_t TreeQueries::ChildNodeCount(const ContainerNode &node) noexcept
  {
    return Krys::HTML::Count(ConstChildNodeRange(node));
  }

  KRYS_NODISCARD size_t TreeQueries::ChildElementCount(const ContainerNode &node) noexcept
  {
    return Krys::HTML::Count(ConstChildElementRange(node));
  }

  bool TreeQueries::HasElementChild(const ContainerNode &node) noexcept
  {
    return std::ranges::any_of(ConstChildNodeRange(node), [](const Node &n) { return Is<Element>(n); });
  }

  void TreeQueries::CollectChildNodes(ContainerNode &parent, SmallNodeList &collection) noexcept
  {
    for (Node &child : ChildNodeRange(parent))
    {
      collection.emplace_back(ShareRef(child));
    }
  }

  void TreeQueries::CollectChildElements(ContainerNode &parent, SmallElementList &collection) noexcept
  {
    for (Element &child : ChildElementRange(parent))
    {
      collection.emplace_back(ShareRef(child));
    }
  }

  List<Ref<Node>> TreeQueries::InclusiveAncestors(Node &node) noexcept
  {
    List<Ref<Node>> collection;
    for (Node &ancestor : InclusiveAncestorRange(node))
    {
      collection.emplace_back(ShareRef(ancestor));
    }

    return collection;
  }

  DOMString TreeQueries::DescendantTextContent(const ContainerNode &node) noexcept
  {
    DOMString content;
    for (const Node &descendant : ConstDescendantRange(node))
    {
      if (auto *textNode = DynamicDowncast<Text>(descendant))
      {
        content += *textNode->TextContent();
      }
    }

    return content;
  }

  DOMString TreeQueries::ChildTextContent(const ContainerNode &node) noexcept
  {
    DOMString content;
    for (const Node &child : ConstChildNodeRange(node))
    {
      if (auto *textNode = DynamicDowncast<Text>(child))
      {
        content += *textNode->TextContent();
      }
    }

    return content;
  }

  DOMString TreeQueries::ContiguousTextContent(const Text &node) noexcept
  {
    RawPtr<const Text> start = &node;
    while (RawPtr<const Node> prev = start->PreviousSibling())
    {
      if (const auto *prevText = DynamicDowncast<Text>(prev))
      {
        start = prevText;
      }
      else
      {
        break;
      }
    }

    DOMString content;
    for (RawPtr<const Node> current = start; current && current->IsTextNode();
         current = current->NextSibling())
    {
      content += *Downcast<Text>(current)->TextContent();
    }
    return content;
  }

  DOMString TreeQueries::ContiguousExclusiveTextContent(const Text &node) noexcept
  {
    RawPtr<const Text> start = &node;
    while (RawPtr<const Node> prev = start->PreviousSibling())
    {
      if (IsExclusiveTextNode(*prev))
      {
        start = Downcast<Text>(prev);
      }
      else
      {
        break;
      }
    }

    DOMString content {};
    for (RawPtr<const Node> current = start; current && IsExclusiveTextNode(*current);
         current = current->NextSibling())
    {
      content += *Downcast<Text>(current)->TextContent();
    }

    return content;
  }

  DOMString TreeQueries::FollowingContiguousExclusiveTextContent(const Text &node) noexcept
  {
    DOMString content {};

    RawPtr<const Node> current = node.NextSibling();
    while (current != nullptr && IsExclusiveTextNode(*current))
    {
      content += *Downcast<Text>(current)->TextContent();
      current = current->NextSibling();
    }

    return content;
  }

  RawPtr<ContainerNode> TreeQueries::CommonAncestorContainer(Node &a, Node &b) noexcept
  {
    if (!HasSameRoot(a, b))
    {
      return nullptr;
    }

    auto *container = &a;
    while (container)
    {
      if (IsInclusiveAncestor(*container, b))
      {
        return Downcast<ContainerNode>(container);
      }

      container = container->ParentNode();
    }

    return nullptr;
  }
}
