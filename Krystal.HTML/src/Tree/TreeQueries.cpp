#include "Krystal.HTML/Tree/TreeQueries.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Tree/TreeTraversal.hpp"

namespace Krys::HTML
{
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
    RawPtr<const Node> current = &a;
    while (current = current->ParentNode())
    {
      if (current == &b)
      {
        return true;
      }
    }

    return false;
  }

  bool TreeQueries::IsInclusiveDescendant(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return true;
    }

    return IsDescendant(a, b);
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

    if (a.ParentNode() == nullptr || b.ParentNode() == nullptr)
    {
      return false;
    }

    return a.ParentNode() == b.ParentNode();
  }

  bool TreeQueries::IsPreceding(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return false;
    }

    RawPtr<const Node> current = &a;
    while (current = TreeTraversal::Next(*current))
    {
      if (current == &b)
      {
        return true;
      }
    }

    return false;
  }

  bool TreeQueries::IsFollowing(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return false;
    }

    RawPtr<const Node> current = &b;
    while (current = TreeTraversal::Next(*current))
    {
      if (current == &a)
      {
        return true;
      }
    }

    return false;
  }

  size_t TreeQueries::Index(const Node &node) noexcept
  {
    size_t index = 0;
    for (auto *sibling = node.PreviousSibling(); sibling; sibling = sibling->PreviousSibling())
    {
      index++;
    }
    return index;
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

  RawPtr<EventTarget> TreeQueries::Retarget(RawPtr<EventTarget> a, EventTarget& b) noexcept
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

  bool TreeQueries::HasSameRoot(const Node &a, const Node &b) noexcept
  {
    return &Root(a) == &Root(b);
  }

  bool TreeQueries::HasSameShadowIncludingRoot(const Node &a, const Node &b) noexcept
  {
    return &ShadowIncludingRoot(a) == &ShadowIncludingRoot(b);
  }

  bool TreeQueries::IsHostIncludingAncestorOf(Node &node, Node &other) noexcept
  {
    // TODO(IMPL):
    return false;
  }

  bool TreeQueries::IsHostIncludingInclusiveAncestorOf(Node &node, Node &other) noexcept
  {
    // TODO(IMPL):
    return false;
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

    RawPtr<Node> current = node;
    while (current = TreeTraversal::Next(*current))
    {
      if (current->IsDocumentTypeNode())
      {
        return true;
      }
    }

    return false;
  }

  bool TreeQueries::IsExclusiveTextNode(const Node &node) noexcept
  {
    return node.IsTextNode() && !node.IsCDATASectionNode();
  }

  bool TreeQueries::IsInclusiveDescendantOf(const ContainerNode &node,
                                            const Node &possibleInclusiveDescendant) noexcept
  {
    if (!HasSameRoot(node, possibleInclusiveDescendant))
    {
      return false;
    }

    RawPtr<const Node> current = &possibleInclusiveDescendant;
    while (current)
    {
      if (current == &node)
      {
        return true;
      }
      current = current->ParentNode();
    }

    return false;
  }

  RawPtr<ShadowRoot> TreeQueries::GetShadowRoot(const Node &node) noexcept
  {
    if (auto *element = DynamicDowncast<Element>(node))
    {
      return element->GetShadowRoot();
    }

    return nullptr;
  }

  RawPtr<Node> TreeQueries::ChildAt(const Node &node, size_t index) noexcept
  {
    size_t currentIndex = 0;
    for (auto child = node.FirstChild(); child; child = child->NextSibling())
    {
      if (currentIndex == index)
      {
        return child;
      }
      currentIndex++;
    }
    return nullptr;
  }

  size_t TreeQueries::ChildNodeCount(const ContainerNode &node) noexcept
  {
    size_t count = 0;
    for (auto child = node.FirstChild(); child; child = child->NextSibling())
    {
      count++;
    }
    return count;
  }

  KRYS_NODISCARD size_t TreeQueries::ChildElementCount(const ContainerNode &node) noexcept
  {
    size_t count = 0;
    for (auto childElement = TreeTraversal::FirstElementChild(node); childElement;
         childElement = TreeTraversal::NextElementSibling(*childElement))
    {
      count++;
    }

    return count;
  }

  bool TreeQueries::HasElementChild(const ContainerNode &node) noexcept
  {
    for (RawPtr<Node> child = node.FirstChild(); child; child = child->NextSibling())
    {
      if (child->IsElementNode())
      {
        return true;
      }
    }

    return false;
  }

  void TreeQueries::CollectChildNodes(const ContainerNode &parent, SmallNodeList &collection) noexcept
  {
    for (auto child = parent.FirstChild(); child; child = child->NextSibling())
    {
      collection.emplace_back(ShareRef(*child));
    }
  }

  void TreeQueries::CollectChildElements(const ContainerNode &parent, SmallElementList &collection) noexcept
  {
    for (auto childElement = TreeTraversal::FirstElementChild(parent); childElement;
         childElement = TreeTraversal::NextElementSibling(*childElement))
    {
      collection.emplace_back(ShareRef(*childElement));
    }
  }

  List<Ref<Node>> TreeQueries::InclusiveAncestors(Node &node) noexcept
  {
    List<Ref<Node>> ancestors;
    for (RawPtr<Node> current = &node; current; current = current->ParentNode())
    {
      ancestors.emplace_back(ShareRef(*current));
    }
    return ancestors;
  }

  DOMString TreeQueries::DescendantTextContent(const ContainerNode &node) noexcept
  {
    DOMString content;

    for (auto *descendant = node.FirstChild(); descendant;
         descendant = TreeTraversal::Next(*descendant, &node))
    {
      if (auto *textNode = DynamicDowncast<Text>(descendant))
      {
        content += textNode->TextContent();
      }
    }

    return content;
  }

  DOMString TreeQueries::ChildTextContent(const ContainerNode &node) noexcept
  {
    DOMString content;
    for (auto child = node.FirstChild(); child; child = child->NextSibling())
    {
      if (auto *textNode = DynamicDowncast<Text>(child))
      {
        content += textNode->TextContent();
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
      content += Downcast<Text>(current)->TextContent();
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

    DOMString content;
    for (RawPtr<const Node> current = start; current && IsExclusiveTextNode(*current);
         current = current->NextSibling())
    {
      content += Downcast<Text>(current)->TextContent();
    }

    return content;
  }

  bool TreeQueries::IsInclusiveAncestorOf(const Node &a, const Node &b) noexcept
  {
    if (!HasSameRoot(a, b))
    {
      return false;
    }

    RawPtr<const Node> current = &b;
    while (current)
    {
      if (current == &a)
      {
        return true;
      }

      current = current->ParentNode();
    }

    return false;
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
      if (IsInclusiveAncestorOf(*container, b))
      {
        return Downcast<ContainerNode>(container);
      }

      container = container->ParentNode();
    }

    return nullptr;
  }
}
