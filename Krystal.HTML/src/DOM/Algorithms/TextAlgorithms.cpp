#include "Krystal.HTML/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/DOM/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/CDATASection.hpp"
#include "Krystal.HTML/DOM/Enums/MutationRecordType.hpp"
#include "Krystal.HTML/DOM/Text.hpp"

namespace Krys::HTML
{
  ExceptionOr<void> TextAlgorithms::Replace(CharacterData &node, size_t offset, size_t count,
                                            DOMString &&data) noexcept
  {
    auto length = node.Length();
    if (offset > length)
    {
      return ExceptionCode::IndexSizeError;
    }

    if (offset + count > length)
    {
      count = length - offset;
    }

    MutationObserverAlgorithms::QueueMutationRecord(MutationRecordType::CharacterData, ShareRef(node),
                                                    DOMStringAtom::Null(), DOMStringAtom::Null(), node._data,
                                                    {}, {}, nullptr, nullptr);
    node._data.insert(offset, data);
    auto deleteOffset = offset + data.size();
    node._data.erase(deleteOffset, count);

    LiveRangeUpdater::CharacterDataReplaced(node, offset, count, data.size());

    if (auto parent = node.ParentNode())
    {
      ExtensibilityHooks::NodeChildrenChanged(*parent);
    }

    return {};
  }

  ExceptionOr<DOMString> TextAlgorithms::Substring(const CharacterData &node, size_t offset,
                                                   size_t count) noexcept
  {
    auto length = node.Length();
    if (offset > length)
    {
      return ExceptionCode::IndexSizeError;
    }

    if (offset + count > length)
    {
      return node._data.substr(offset);
    }

    return node._data.substr(offset, count);
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
      if (!TreeQueries::IsExclusiveTextNode(*prev))
      {
        break;
      }

      start = prev;
    }

    SmallTextNodeList nodes;
    for (auto *current = start; TreeQueries::IsExclusiveTextNode(current); current = current->NextSibling())
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
      if (!TreeQueries::IsExclusiveTextNode(*prev))
      {
        break;
      }

      start = prev;
    }

    SmallConstTextNodeList nodes;
    for (auto *current = start; TreeQueries::IsExclusiveTextNode(current); current = current->NextSibling())
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
      return ExceptionCode::IndexSizeError;
    }

    auto count = length - offset;
    auto newData = Substring(node, offset, count);

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