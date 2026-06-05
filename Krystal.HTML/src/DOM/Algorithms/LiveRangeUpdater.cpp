#include "Krystal.HTML/DOM/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/CharacterData.hpp"
#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include <cassert>

namespace Krys::HTML
{
  void LiveRangeUpdater::Created(Range &range) noexcept
  {
    range.StartContainer()->NodeDocument().LiveRanges().push_back(&range);
  }

  void LiveRangeUpdater::Destroyed(Range &range) noexcept
  {
    auto &liveRanges = range.StartContainer()->NodeDocument().LiveRanges();
    std::erase_if(liveRanges, [&range](const auto *liveRange) { return liveRange == &range; });
  }

  void LiveRangeUpdater::RootChanged(Document &oldDocument, Document &newDocument, Range &range) noexcept
  {
    std::erase_if(oldDocument.LiveRanges(), [&range](const auto *liveRange) { return liveRange == &range; });
    newDocument.LiveRanges().push_back(&range);
  }

  void LiveRangeUpdater::PreRemove(const Node &node) noexcept
  {
    auto *parent = node.ParentNode();
    assert(parent != nullptr);

    auto index = TreeQueries::Index(node);

    for (auto &range : node.NodeDocument().LiveRanges())
    {
      auto *originalStartContainer = range->StartContainer();
      auto *originalEndContainer = range->EndContainer();

      auto originalStartOffset = range->StartOffset();
      auto originalEndOffset = range->EndOffset();

      if (auto *startContainer = DynamicDowncast<ContainerNode>(*originalStartContainer))
      {
        if (TreeQueries::IsInclusiveDescendant(*startContainer, node))
        {
          range->_start = BoundaryPoint {ShareRef(*parent), index};
        }
      }

      if (auto *endContainer = DynamicDowncast<ContainerNode>(*originalEndContainer))
      {
        if (TreeQueries::IsInclusiveDescendant(*endContainer, node))
        {
          range->_end = BoundaryPoint {ShareRef(*parent), index};
        }
      }

      if (originalStartContainer == parent && originalStartOffset > index)
      {
        range->_start.Offset -= 1;
      }

      if (originalEndContainer == parent && originalEndOffset > index)
      {
        range->_end.Offset -= 1;
      }
    }
  }

  void LiveRangeUpdater::CharacterDataReplaced(CharacterData &characterData, size_t offset, size_t count,
                                               size_t newDataSize) noexcept
  {
    for (auto &range : characterData.NodeDocument().LiveRanges())
    {
      auto *originalStartContainer = range->StartContainer();
      auto *originalEndContainer = range->EndContainer();

      auto originalStartOffset = range->StartOffset();
      auto originalEndOffset = range->EndOffset();

      if (originalStartContainer == &characterData)
      {
        if (originalStartOffset > offset && originalStartOffset <= offset + count)
        {
          range->_start.Offset = offset;
        }
        else if (originalStartOffset > offset + count)
        {
          range->_start.Offset += newDataSize;
          range->_start.Offset -= count;
        }
      }

      if (originalEndContainer == &characterData)
      {
        if (originalEndOffset > offset && originalEndOffset <= offset + count)
        {
          range->_end.Offset = offset;
        }
        else if (originalEndOffset > offset + count)
        {
          range->_end.Offset += newDataSize;
          range->_end.Offset -= count;
        }
      }
    }
  }

  void LiveRangeUpdater::NodeNormalized(Node &node, Text &removedText, size_t length) noexcept
  {
    auto *parent = removedText.ParentNode();

    auto index = TreeQueries::Index(removedText);
    for (auto &range : node.NodeDocument().LiveRanges())
    {
      auto *originalStartContainer = range->StartContainer();
      auto *originalEndContainer = range->EndContainer();

      auto originalStartOffset = range->StartOffset();
      auto originalEndOffset = range->EndOffset();

      if (originalStartContainer == &removedText)
      {
        range->_start = BoundaryPoint {ShareRef(node), range->_start.Offset + length};
      }

      if (originalEndContainer == &removedText)
      {
        range->_end = BoundaryPoint {ShareRef(node), range->_end.Offset + length};
      }

      if (originalStartContainer == parent && originalStartOffset == index)
      {
        range->_start = BoundaryPoint {ShareRef(node), length};
      }

      if (originalEndContainer == parent && originalEndOffset == index)
      {
        range->_end = BoundaryPoint {ShareRef(node), length};
      }
    }
  }

  void LiveRangeUpdater::MovedBeforeNode(Node &movedBefore) noexcept
  {
    auto childIndex = TreeQueries::Index(movedBefore);
    auto *newParent = movedBefore.ParentNode();
    assert(newParent != nullptr);

    for (auto range : movedBefore.NodeDocument().LiveRanges())
    {
      auto *originalStartContainer = range->StartContainer();
      auto *originalEndContainer = range->EndContainer();

      auto originalStartOffset = range->StartOffset();
      auto originalEndOffset = range->EndOffset();

      if (originalStartContainer == newParent && originalStartOffset > childIndex)
      {
        range->_start.Offset += 1;
      }

      if (originalEndContainer == newParent && originalEndOffset > childIndex)
      {
        range->_end.Offset += 1;
      }
    }
  }

  void LiveRangeUpdater::InsertedBeforeNode(Node &insertedBefore, size_t nodesInserted) noexcept
  {
    auto *newParent = insertedBefore.ParentNode();
    assert(newParent != nullptr);

    auto childIndex = TreeQueries::Index(insertedBefore);
    for (auto &range : newParent->NodeDocument().LiveRanges())
    {
      auto *originalStartContainer = range->StartContainer();
      auto *originalEndContainer = range->EndContainer();

      auto originalStartOffset = range->StartOffset();
      auto originalEndOffset = range->EndOffset();

      if (originalStartContainer == newParent && originalStartOffset > childIndex)
      {
        range->_start.Offset += nodesInserted;
      }

      if (originalEndContainer == newParent && originalEndOffset > childIndex)
      {
        range->_end.Offset += nodesInserted;
      }
    }
  }

  void LiveRangeUpdater::SplitTextNode(Text &original, Text &newNode, size_t offset) noexcept
  {
    auto index = TreeQueries::Index(original);

    for (auto &range : original.NodeDocument().LiveRanges())
    {
      auto *originalStartContainer = range->StartContainer();
      auto *originalEndContainer = range->EndContainer();

      auto originalStartOffset = range->StartOffset();
      auto originalEndOffset = range->EndOffset();

      if (originalStartContainer == &original && originalStartOffset > offset)
      {
        range->_start = BoundaryPoint {ShareRef(newNode), originalStartOffset - offset};
      }

      if (originalEndContainer == &original && originalEndOffset > offset)
      {
        range->_end = BoundaryPoint {ShareRef(newNode), originalEndOffset - offset};
      }

      if (originalStartContainer == original.ParentNode() && originalStartOffset == index + 1uz)
      {
        range->_start.Offset += 1;
      }

      if (originalEndContainer == original.ParentNode() && originalEndOffset == index + 1uz)
      {
        range->_end.Offset += 1;
      }
    }
  }
}