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
  void LiveRangeUpdater::PreRemove(const Node &node) noexcept
  {
    auto *parent = node.ParentNode();
    assert(parent != nullptr);

    auto index = TreeQueries::Index(node);

    for (auto &range : node.NodeDocument().LiveRanges())
    {
      if (auto *startContainer = DynamicDowncast<ContainerNode>(*range->StartContainer()))
      {
        if (TreeQueries::IsInclusiveDescendant(*startContainer, node))
        {
          range->SetStart(*parent, index);
        }
      }

      if (auto *endContainer = DynamicDowncast<ContainerNode>(*range->EndContainer()))
      {
        if (TreeQueries::IsInclusiveDescendant(*endContainer, node))
        {
          range->SetEnd(*parent, index);
        }
      }

      if (range->StartContainer() == parent)
      {
        range->SetStart(*parent, range->StartOffset() - 1uz);
      }

      if (range->EndContainer() == parent)
      {
        range->SetEnd(*parent, range->EndOffset() - 1uz);
      }
    }
  }

  void LiveRangeUpdater::CharacterDataReplaced(CharacterData &characterData, size_t offset, size_t count,
                                               size_t newDataSize) noexcept
  {
    for (auto &range : characterData.NodeDocument().LiveRanges())
    {
      if (range->StartContainer() == &characterData)
      {
        if (range->StartOffset() > offset && range->StartOffset() <= offset + count)
        {
          range->SetStart(characterData, offset);
        }
        else if (range->StartOffset() > offset + count)
        {
          range->SetStart(characterData, range->StartOffset() + newDataSize - count);
        }
      }

      if (range->EndContainer() == &characterData)
      {
        if (range->EndOffset() > offset && range->EndOffset() <= offset + count)
        {
          range->SetEnd(characterData, offset);
        }
        else if (range->EndOffset() > offset + count)
        {
          range->SetEnd(characterData, range->EndOffset() + newDataSize - count);
        }
      }
    }
  }

  void LiveRangeUpdater::NodeNormalized(Node &node, Text &removedText, size_t length) noexcept
  {
    for (auto &range : node.NodeDocument().LiveRanges())
    {
      if (range->StartContainer() == &removedText)
      {
        range->SetStart(node, range->StartOffset() + length);
      }

      if (range->EndContainer() == &removedText)
      {
        range->SetEnd(node, range->EndOffset() + length);
      }

      if (range->StartContainer() == removedText.ParentNode()
          && range->StartOffset() == TreeQueries::Index(removedText))
      {
        range->SetStart(node, length);
      }

      if (range->EndContainer() == removedText.ParentNode()
          && range->EndOffset() == TreeQueries::Index(removedText))
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

    for (auto range : newParent->NodeDocument().LiveRanges())
    {
      if (range->StartContainer() == newParent && range->StartOffset() > childIndex)
      {
        range->SetStart(*newParent, range->StartOffset() + 1);
      }

      if (range->EndContainer() == newParent && range->EndOffset() > childIndex)
      {
        range->SetEnd(*newParent, range->EndOffset() + 1);
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
      if (range->StartContainer() == newParent && range->StartOffset() > childIndex)
      {
        range->SetStart(*newParent, range->StartOffset() + nodesInserted);
      }

      if (range->EndContainer() == newParent && range->EndOffset() > childIndex)
      {
        range->SetEnd(*newParent, range->EndOffset() + nodesInserted);
      }
    }
  }

  void LiveRangeUpdater::SplitTextNode(Text &original, Text &newNode, size_t offset) noexcept
  {
    auto index = TreeQueries::Index(original);
    for (auto &range : original.NodeDocument().LiveRanges())
    {
      if (range->StartContainer() == &original && range->StartOffset() > offset)
      {
        range->SetStart(newNode, range->StartOffset() - offset);
      }

      if (range->EndContainer() == &original)
      {
        range->SetEnd(newNode, range->EndOffset() - offset);
      }

      if (range->StartContainer() == original.ParentNode() && range->StartOffset() == index + 1uz)
      {
        range->SetStart(*range->StartContainer(), range->StartOffset() + 1uz);
      }

      if (range->EndContainer() == original.ParentNode() && range->EndOffset() == index + 1uz)
      {
        range->SetEnd(*range->EndContainer(), range->EndOffset() + 1uz);
      }
    }
  }
}