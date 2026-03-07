#include "Krystal.HTML/Tree/TreeQueries.hpp"
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

  bool TreeQueries::HasSameRoot(const Node &a, const Node &b) noexcept
  {
    return &Root(a) == &Root(b);
  }

  bool TreeQueries::HasSameShadowIncludingRoot(const Node &a, const Node &b) noexcept
  {
    return &ShadowIncludingRoot(a) == &ShadowIncludingRoot(b);
  }

  bool TreeQueries::IsPreceding(const Node &a, const Node &b) noexcept
  {
    // TODO(IMPL):
    return false;
  }

  bool TreeQueries::IsFollowing(const Node &a, const Node &b) noexcept
  {
    if (!HasSameRoot(a, b))
    {
      return false;
    }

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

  bool TreeQueries::IsAncestorOf(const Node &a, const Node &b) noexcept
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

  bool TreeQueries::IsChildOf(const Node &parent, const Node &child) noexcept
  {
    if (!HasSameRoot(parent, child))
    {
      return false;
    }

    return child.ParentNode() == &parent;
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

  size_t TreeQueries::NodeIndex(const Node &node) noexcept
  {
    size_t index = 0;
    for (auto *sibling = node.PreviousSibling(); sibling; sibling = sibling->PreviousSibling())
    {
      index++;
    }
    return index;
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
