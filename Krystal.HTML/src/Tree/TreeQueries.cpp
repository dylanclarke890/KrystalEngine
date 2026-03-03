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
  bool TreeQueries::HasSameRoot(const Node &a, const Node &b) noexcept
  {
    return &a.Root() == &b.Root();
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

  RawPtr<ShadowRoot> TreeQueries::GetShadowRoot(const Node &node) noexcept
  {
    if (auto *element = DynamicDowncast<Element>(node))
    {
      return element->GetShadowRoot();
    }

    return nullptr;
  }

  size_t TreeQueries::ChildNodeCount(const ContainerNode &node) noexcept
  {
    size_t count = 0;
    for (auto child = ShareRefPtr(node.FirstChild()); child; child = ShareRefPtr(child->NextSibling()))
    {
      count++;
    }
    return count;
  }

  KRYS_NODISCARD size_t TreeQueries::ChildElementCount(const ContainerNode &node) noexcept
  {
    size_t count = 0;
    for (auto childElement = ShareRefPtr(TreeTraversal::FirstElementChild(node)); childElement;
         childElement = ShareRefPtr(TreeTraversal::NextElementSibling(*childElement)))
    {
      count++;
    }

    return count;
  }

  void TreeQueries::CollectChildNodes(const ContainerNode &parent, SmallNodeList &collection) noexcept
  {
    for (auto child = ShareRefPtr(parent.FirstChild()); child; child = ShareRefPtr(child->NextSibling()))
    {
      collection.emplace_back(ShareRef(*child));
    }
  }

  void TreeQueries::CollectChildElements(const ContainerNode &parent, SmallElementList &collection) noexcept
  {
    for (auto childElement = ShareRefPtr(TreeTraversal::FirstElementChild(parent)); childElement;
         childElement = ShareRefPtr(TreeTraversal::NextElementSibling(*childElement)))
    {
      collection.emplace_back(ShareRef(*childElement));
    }
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
    for (auto child = ShareRefPtr(node.FirstChild()); child; child = ShareRefPtr(child->NextSibling()))
    {
      if (auto *textNode = DynamicDowncast<Text>(*child))
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
      if (const auto *prevText = DynamicDowncast<Text>(*prev))
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
}
