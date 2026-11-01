#include "Krystal.Booey/DOM/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include <cassert>

namespace krys::boo::dom
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
    // 1. Let parent be node's parent.
    auto *parent = node.ParentNode();
    // 2. Assert: parent is non-null.
    krys_debug_assert(parent != nullptr);

    // 3. Let index be node's index.
    auto index = TreeQueries::Index(node);

    for (auto &range : node.NodeDocument().LiveRanges())
    {
      auto *originalStartContainer = range->StartContainer();
      auto *originalEndContainer = range->EndContainer();

      auto originalStartOffset = range->StartOffset();
      auto originalEndOffset = range->EndOffset();

      // 4. For each live range whose start node is an inclusive descendant of node, set its start to (parent,
      //    index).
      if (TreeQueries::IsInclusiveDescendant(*originalStartContainer, node))
      {
        range->_start = BoundaryPoint {ShareRef(*parent), index};
      }

      // 5. For each live range whose end node is an inclusive descendant of node, set its end to (parent,
      //    index).
      if (TreeQueries::IsInclusiveDescendant(*originalEndContainer, node))
      {
        range->_end = BoundaryPoint {ShareRef(*parent), index};
      }

      // 6. For each live range whose start node is parent and start offset is greater than index, decrease
      //    its start offset by 1.
      if (originalStartContainer == parent && originalStartOffset > index)
      {
        range->_start.Offset -= 1;
      }

      // 7. For each live range whose end node is parent and end offset is greater than index, decrease its
      //    end offset by 1.
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
        // 8. For each live range whose start node is node and start offset is greater than offset but less
        //    than or equal to offset + count: set its start offset to offset.
        if (originalStartOffset > offset && originalStartOffset <= offset + count)
        {
          range->_start.Offset = offset;
        }
        // 10. For each live range whose start node is node and start offset is greater than offset + count:
        //     increase its start offset by data's length and decrease it by count.
        else if (originalStartOffset > offset + count)
        {
          range->_start.Offset += newDataSize;
          range->_start.Offset -= count;
        }
      }

      if (originalEndContainer == &characterData)
      {
        // 9. For each live range whose end node is node and end offset is greater than offset but less than
        // or
        //    equal to offset + count: set its end offset to offset.
        if (originalEndOffset > offset && originalEndOffset <= offset + count)
        {
          range->_end.Offset = offset;
        }
        // 11. For each live range whose end node is node and end offset is greater than offset + count:
        //     increase its end offset by data's length and decrease it by count.
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
    krys_debug_assert(newParent != nullptr);

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
    krys_debug_assert(newParent != nullptr);

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

      // 7b. For each live range whose start node is node and start offset is greater than offset, set its
      //     start node to newNode and decrease its start offset by offset.
      if (originalStartContainer == &original && originalStartOffset > offset)
      {
        range->_start = BoundaryPoint {ShareRef(newNode), originalStartOffset - offset};
      }

      // 7c. For each live range whose end node is node and end offset is greater than offset, set its end
      //     node to newNode and decrease its end offset by offset.
      if (originalEndContainer == &original && originalEndOffset > offset)
      {
        range->_end = BoundaryPoint {ShareRef(newNode), originalEndOffset - offset};
      }

      // 7d. For each live range whose start node is parent and start offset is equal to the index of node
      //     plus 1, increase its start offset by 1.
      if (originalStartContainer == original.ParentNode() && originalStartOffset == index + 1uz)
      {
        range->_start.Offset += 1;
      }

      // 7e. For each live range whose end node is parent and end offset is equal to the index of node plus
      //     1, increase its end offset by 1.
      if (originalEndContainer == original.ParentNode() && originalEndOffset == index + 1uz)
      {
        range->_end.Offset += 1;
      }
    }
  }
}