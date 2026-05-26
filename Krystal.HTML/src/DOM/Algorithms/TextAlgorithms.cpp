#include "Krystal.HTML/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CDATASection.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"

namespace Krys::HTML
{
  bool TextAlgorithms::IsExclusiveTextNode(const Node &node) noexcept
  {
    return Is<Text>(node) && !Is<CDATASection>(node);
  }

  bool TextAlgorithms::IsExclusiveTextNode(RawPtr<const Node> node) noexcept
  {
    return Is<Text>(node) && !Is<CDATASection>(node);
  }

  SmallTextNodeList TextAlgorithms::ContiguousTextNodes(Text &node) noexcept
  {
    RawPtr<Node> start = &node;
    while (auto *prev = start->PreviousSibling())
    {
      if (!Is<Text>(prev))
      {
        break;
      }

      start = prev;
    }

    SmallTextNodeList nodes;
    for (auto *current = start; Is<Text>(current); current = current->NextSibling())
    {
      nodes.push_back(ShareRef(*Downcast<Text>(current)));
    }

    return nodes;
  }

  SmallConstTextNodeList TextAlgorithms::ContiguousTextNodes(const Text &node) noexcept
  {
    RawPtr<const Node> start = &node;
    while (auto *prev = start->PreviousSibling())
    {
      if (!Is<Text>(prev))
      {
        break;
      }

      start = prev;
    }

    SmallConstTextNodeList nodes;
    for (auto *current = start; Is<Text>(current); current = current->NextSibling())
    {
      nodes.push_back(ShareRef(*Downcast<Text>(current)));
    }

    return nodes;
  }

  SmallTextNodeList TextAlgorithms::ContiguousExclusiveTextNodes(Text &node) noexcept
  {
    RawPtr<Node> start = &node;
    while (auto *prev = start->PreviousSibling())
    {
      if (!IsExclusiveTextNode(*prev))
      {
        break;
      }

      start = prev;
    }

    SmallTextNodeList nodes;
    for (auto *current = start; IsExclusiveTextNode(current); current = current->NextSibling())
    {
      nodes.push_back(ShareRef(*Downcast<Text>(current)));
    }

    return nodes;
  }

  SmallConstTextNodeList TextAlgorithms::ContiguousExclusiveTextNodes(const Text &node) noexcept
  {
    RawPtr<const Node> start = &node;
    while (auto *prev = start->PreviousSibling())
    {
      if (!IsExclusiveTextNode(*prev))
      {
        break;
      }

      start = prev;
    }

    SmallConstTextNodeList nodes;
    for (auto *current = start; IsExclusiveTextNode(current); current = current->NextSibling())
    {
      nodes.push_back(ShareRef(*Downcast<Text>(current)));
    }

    return nodes;
  }

  DOMString TextAlgorithms::ChildTextContent(const ContainerNode &node) noexcept
  {
    DOMString content {};
    for (auto &child : ConstChildNodeRange(node))
    {
      if (auto *textNode = DynamicDowncast<Text>(child))
      {
        content += *textNode->TextContent();
      }
    }

    return content;
  }

  DOMString TextAlgorithms::DescendantTextContent(const ContainerNode &node) noexcept
  {
    DOMString content;
    for (auto &descendant : ConstDescendantRange(node))
    {
      if (auto *textNode = DynamicDowncast<Text>(descendant))
      {
        content += *textNode->TextContent();
      }
    }

    return content;
  }

  ExceptionOr<Ref<Text>> TextAlgorithms::Split(Text &node, size_t offset) noexcept
  {
    auto length = node.Length();
    if (offset > length)
    {
      return Exception {ExceptionCode::IndexSizeError};
    }

    auto count = length - offset;
    // TODO(impl): CHARACTER-DATA: use the character data algorithms instead of using the functions directly.
    auto newData = node.SubstringData(offset, count);
    if (newData.HasException())
    {
      return newData.ReleaseException();
    }

    auto newNode = CreateRef<Text>(node.NodeDocument(), newData.ReleaseValue());
    if (auto parent = ShareRefPtr(node.ParentNode()))
    {
      if (auto insertResult = parent->InsertBefore(*newNode, node.NextSibling()); insertResult.HasException())
      {
        return insertResult.ReleaseException();
      }

      LiveRangeUpdater::SplitTextNode(node, *newNode, offset);
    }

    node.ReplaceData(offset, count, u8"");

    return newNode;
  }
}