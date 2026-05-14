#include "Krystal.HTML/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include <cassert>

namespace Krys::HTML
{
  void LiveRangeUpdater::Add(Range &range) noexcept
  {
    range.StartContainer()->NodeDocument().LiveRanges().push_back(&range);
  }

  void LiveRangeUpdater::Remove(Range &range) noexcept
  {
    auto &liveRanges = range.StartContainer()->NodeDocument().LiveRanges();
    std::erase_if(liveRanges, [&range](const auto *liveRange) { return liveRange == &range; });
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
          range->SetStart(*parent, index);
        }
      }

      if (auto *endContainer = DynamicDowncast<ContainerNode>(*originalEndContainer))
      {
        if (TreeQueries::IsInclusiveDescendant(*endContainer, node))
        {
          range->SetEnd(*parent, index);
        }
      }

      if (originalStartContainer == parent)
      {
        range->SetStart(*parent, originalStartOffset - 1uz);
      }

      if (originalEndContainer == parent)
      {
        range->SetEnd(*parent, originalEndOffset - 1uz);
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
          range->SetStart(characterData, offset);
        }
        else if (originalStartOffset > offset + count)
        {
          range->SetStart(characterData, originalStartOffset + newDataSize - count);
        }
      }

      if (originalEndContainer == &characterData)
      {
        if (originalEndOffset > offset && originalEndOffset <= offset + count)
        {
          range->SetEnd(characterData, offset);
        }
        else if (originalEndOffset > offset + count)
        {
          range->SetEnd(characterData, originalEndOffset + newDataSize - count);
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
        range->SetStart(node, originalStartOffset + length);
      }

      if (originalEndContainer == &removedText)
      {
        range->SetEnd(node, originalEndOffset + length);
      }

      if (originalStartContainer == parent && originalStartOffset == index)
      {
        range->SetStart(node, length);
      }

      if (originalEndContainer == parent && originalEndOffset == index)
      {
        range->SetEnd(node, length);
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
        range->SetStart(*newParent, originalStartOffset + 1uz);
      }

      if (originalEndContainer == newParent && originalEndOffset > childIndex)
      {
        range->SetEnd(*newParent, originalEndOffset + 1uz);
      }
    }
  }

  void LiveRangeUpdater::InsertedBeforeNode(Node &insertedBefore, size_t nodesInserted) noexcept
  {
    auto childIndex = TreeQueries::Index(insertedBefore);
    auto *newParent = insertedBefore.ParentNode();
    assert(newParent != nullptr);

    for (auto &range : newParent->NodeDocument().LiveRanges())
    {
      auto *originalStartContainer = range->StartContainer();
      auto *originalEndContainer = range->EndContainer();

      auto originalStartOffset = range->StartOffset();
      auto originalEndOffset = range->EndOffset();

      if (originalStartContainer == newParent && originalStartOffset > childIndex)
      {
        range->SetStart(*newParent, originalStartOffset + nodesInserted);
      }

      if (originalEndContainer == newParent && originalEndOffset > childIndex)
      {
        range->SetEnd(*newParent, originalEndOffset + nodesInserted);
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
        range->SetStart(newNode, originalStartOffset - offset);
      }

      if (originalEndContainer == &original && originalEndOffset > offset)
      {
        range->SetEnd(newNode, originalEndOffset - offset);
      }

      if (originalStartContainer == original.ParentNode() && originalStartOffset == index + 1uz)
      {
        range->SetStart(*original.ParentNode(), originalStartOffset + 1uz);
      }

      if (originalEndContainer == original.ParentNode() && originalEndOffset == index + 1uz)
      {
        range->SetEnd(*original.ParentNode(), originalEndOffset + 1uz);
      }
    }
  }
}