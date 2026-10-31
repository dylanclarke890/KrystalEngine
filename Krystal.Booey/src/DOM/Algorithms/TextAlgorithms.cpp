#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.Booey/DOM/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Enums/MutationRecordType.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Style/ChildChangeInvalidation.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  ExceptionOr<void> TextAlgorithms::Replace(CharacterData &node, size_t offset, size_t count,
                                            DOMString &&data) noexcept
  {
    // 1. Let length be node's length.
    auto length = node.Length();
    // 2. If offset is greater than length, then throw an "IndexSizeError" DOMException.
    if (offset > length)
    {
      return ExceptionCode::IndexSizeError;
    }

    // 3. If offset + count is greater than length, then set count to length - offset.
    //    (Compared as count > length - offset, which can't overflow.)
    if (count > length - offset)
    {
      count = length - offset;
    }

    // 4. Queue a mutation record of "characterData" for node with null, null, node's data, « », « », null,
    //    and null.
    MutationObserverAlgorithms::QueueMutationRecord(MutationRecordType::CharacterData, ShareRef(node),
                                                    DOMStringAtom::Null(), DOMStringAtom::Null(), node._data,
                                                    {}, {}, nullptr, nullptr);
    {
      // The change to the parent's children that style invalidation sees.
      auto *parent = node.ParentNode();
      ContainerNode::ChildChange childChange {
        ContainerNode::ChildChange::Type::TextChanged, nullptr,
        TreeQueries::PreviousElementSibling(node),     TreeQueries::NextElementSibling(node),
        ContainerNode::ChildChange::Source::API,       ContainerNode::ChildChange::AffectsElements::No};
      Maybe<style::ChildChangeInvalidation> styleInvalidation;
      if (parent)
      {
        styleInvalidation.emplace(*parent, childChange);
      }

      // 5. Insert data into node's data after offset code units.
      node._data.insert(offset, data);
      // 6. Let deleteOffset be offset + data's length.
      auto deleteOffset = offset + data.size();
      // 7. Starting from deleteOffset code units, remove count code units from node's data.
      node._data.erase(deleteOffset, count);
    }

    // 8.-11. Update the live ranges whose start or end node is node.
    LiveRangeUpdater::CharacterDataReplaced(node, offset, count, data.size());
    node.NodeDocument().Selection().TextWasReplaced(
      node, static_cast<uint32>(offset), static_cast<uint32>(count), static_cast<uint32>(data.size()));

    if (auto *text = DynamicDowncast<Text>(node))
    {
      text->UpdateRendererAfterContentChange(offset, count);
    }
    // 12. If node is a ProcessingInstruction node and piAttributesAlreadyUpdated is false, then update
    //     attributes from data given node.
    // FIXME: ProcessingInstruction has no attribute map, so there's nothing to update (and no
    // piAttributesAlreadyUpdated); an xml-stylesheet's sheet is checked again from its data.
    else if (auto *processingInstruction = DynamicDowncast<ProcessingInstruction>(node))
    {
      processingInstruction->CheckStyleSheet();
    }

    // 13. If node's parent is non-null, then run the children changed steps for node's parent.
    if (auto parent = node.ParentNode())
    {
      ExtensibilityHooks::NodeChildrenChanged(*parent);
    }

    return {};
  }

  ExceptionOr<DOMString> TextAlgorithms::Substring(const CharacterData &node, size_t offset,
                                                   size_t count) noexcept
  {
    // 1. Let length be node's length.
    auto length = node.Length();
    // 2. If offset is greater than length, then throw an "IndexSizeError" DOMException.
    if (offset > length)
    {
      return ExceptionCode::IndexSizeError;
    }

    // 3. If offset + count is greater than length, then return a string whose value is the code units from
    //    the offsetth code unit to the end of node's data.
    if (count > length - offset)
    {
      return node._data.substr(offset);
    }

    // 4. Return a string whose value is the code units from the offsetth code unit to the offset+countth code
    //    unit in node's data.
    return node._data.substr(offset, count);
  }

  SmallTextNodeList TextAlgorithms::ContiguousTextNodes(Text &node) noexcept
  {
    Node *start = &node;
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
    const Node *start = &node;
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
    Node *start = &node;
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
    const Node *start = &node;
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