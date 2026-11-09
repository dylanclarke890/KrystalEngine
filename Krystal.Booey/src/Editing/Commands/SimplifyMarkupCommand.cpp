/*
 * Copyright (C) 2012-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Editing/Commands/SimplifyMarkupCommand.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Difference.hpp"

namespace krys::boo
{
  SimplifyMarkupCommand::SimplifyMarkupCommand(dom::Document &document, dom::Node *firstNode,
                                               dom::Node *nodeAfterLast) noexcept
      : CompositeEditCommand(document), _firstNode(ShareRefPtr(firstNode)),
        _nodeAfterLast(ShareRefPtr(nodeAfterLast))
  {
  }

  Ref<SimplifyMarkupCommand> SimplifyMarkupCommand::Create(dom::Document &document, dom::Node *firstNode,
                                                           dom::Node *nodeAfterLast) noexcept
  {
    return CreateRef<SimplifyMarkupCommand>(document, firstNode, nodeAfterLast);
  }

  void SimplifyMarkupCommand::DoApply() noexcept
  {
    RefPtr<dom::ContainerNode> rootNode = ShareRefPtr(_firstNode->ParentNode());
    List<Ref<dom::Node>> nodesToRemove;

    (void)GetDocument().UpdateLayoutIgnorePendingStylesheets();

    // Walk through the inserted nodes, to see if there are elements that could be removed
    // without affecting the style. The goal is to produce leaner markup even when starting
    // from a verbose fragment.
    // We look at inline elements as well as non top level divs that don't have attributes.
    for (auto *node = _firstNode.get(); node && node != _nodeAfterLast; node = dom::TreeQueries::Next(*node))
    {
      if (node->FirstChild() || (Is<dom::Text>(*node) && node->NextSibling()) || !node->ParentNode())
      {
        continue;
      }

      RefPtr<dom::ContainerNode> startingNode = ShareRefPtr(node->ParentNode());
      const auto *startingStyle = startingNode->GetRenderStyle();
      if (!startingStyle)
      {
        continue;
      }
      RefPtr<dom::ContainerNode> currentNode = startingNode;
      RefPtr<dom::Node> topNodeWithStartingStyle;
      while (currentNode != rootNode)
      {
        // FIXME: The simplification algorithm should be rewritten to eliminate redundant
        // parents in cases where the children affect rendered content, as observed with
        // <span><picture></picture></span>.
        if (IsHTMLElementOfType(currentNode.get(), html::TagName::picture))
        {
          break;
        }

        if (rootNode != currentNode->ParentNode() && IsRemovableBlock(currentNode.get()))
        {
          nodesToRemove.push_back(ShareRef<dom::Node>(*currentNode));
        }

        currentNode = ShareRefPtr(currentNode->ParentNode());
        if (!currentNode)
        {
          break;
        }

        auto *renderInline = DynamicDowncast<RenderInline>(currentNode->Renderer());
        if (!renderInline || renderInline->MayAffectLayout())
        {
          continue;
        }

        if (currentNode->FirstChild() != currentNode->LastChild())
        {
          topNodeWithStartingStyle = nullptr;
          break;
        }

        if (style::GetDifference(*currentNode->GetRenderStyle(), *startingStyle)
            == style::DifferenceResult::Equal)
        {
          topNodeWithStartingStyle = currentNode;
        }
      }
      if (topNodeWithStartingStyle)
      {
        for (dom::Node *ancestor = startingNode.get(); ancestor && ancestor != topNodeWithStartingStyle;
             ancestor = ancestor->ParentNode())
        {
          nodesToRemove.push_back(ShareRef(*ancestor));
        }
      }
    }

    // we perform all the DOM mutations at once.
    for (size_t i = 0; i < nodesToRemove.size(); ++i)
    {
      // FIXME: We can do better by directly moving children from nodesToRemove[i].
      int32 numPrunedAncestors = PruneSubsequentAncestorsToRemove(nodesToRemove, i);
      if (numPrunedAncestors < 0)
      {
        continue;
      }
      RemoveNodePreservingChildren(*nodesToRemove[i], AssumeContentIsAlwaysEditable);
      i += static_cast<size_t>(numPrunedAncestors);
    }
  }

  int32 SimplifyMarkupCommand::PruneSubsequentAncestorsToRemove(List<Ref<dom::Node>> &nodesToRemove,
                                                                size_t startNodeIndex) noexcept
  {
    size_t pastLastNodeToRemove = startNodeIndex + 1;
    for (; pastLastNodeToRemove < nodesToRemove.size(); ++pastLastNodeToRemove)
    {
      if (nodesToRemove[pastLastNodeToRemove - 1]->ParentNode() != nodesToRemove[pastLastNodeToRemove].get())
      {
        break;
      }
      if (nodesToRemove[pastLastNodeToRemove]->FirstChild()
          != nodesToRemove[pastLastNodeToRemove]->LastChild())
      {
        break;
      }
    }

    Ref<dom::Node> highestAncestorToRemove = nodesToRemove[pastLastNodeToRemove - 1];
    RefPtr<dom::ContainerNode> parent = ShareRefPtr(highestAncestorToRemove->ParentNode());
    if (!parent) // Parent has already been removed.
    {
      return -1;
    }

    if (pastLastNodeToRemove == startNodeIndex + 1)
    {
      return 0;
    }

    RemoveNode(*nodesToRemove[startNodeIndex], AssumeContentIsAlwaysEditable);
    (void)InsertNodeBefore(*nodesToRemove[startNodeIndex], *highestAncestorToRemove,
                           AssumeContentIsAlwaysEditable);
    RemoveNode(*highestAncestorToRemove, AssumeContentIsAlwaysEditable);

    return static_cast<int32>(pastLastNodeToRemove - startNodeIndex - 1);
  }
}
