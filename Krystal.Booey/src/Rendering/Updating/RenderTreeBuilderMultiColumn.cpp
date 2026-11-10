/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2007 David Smith (catfish.man@gmail.com)
 * Copyright (C) 2003-2024 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderMultiColumn.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderTextControl.hpp"
#include "Krystal.Booey/Rendering/Iterators/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnFlow.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnSet.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnSpannerPlaceholder.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlock.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  // Guards against processing spanners while they're moved back to their placeholders, or out of the flow.
  static bool s_restoringColumnSpannersForContainer = false;
  static bool s_shiftingColumnSpanner = false;

  static RenderMultiColumnSet *FindColumnSetRendering(const RenderMultiColumnFlow &fragmentedFlow,
                                                      const RenderObject &renderer) noexcept
  {
    // Find the set inside which the specified renderer would be rendered.
    for (auto *multicolSet = fragmentedFlow.FirstMultiColumnSet(); multicolSet;
         multicolSet = multicolSet->NextSiblingMultiColumnSet())
    {
      if (multicolSet->ContainsRendererInFragmentedFlow(renderer))
      {
        return multicolSet;
      }
    }
    return nullptr;
  }

  static RenderObject *SpannerPlaceholderCandidate(const RenderObject &renderer,
                                                   const RenderMultiColumnFlow &stayWithin) noexcept
  {
    // Spanner candidate is a next sibling/ancestor's next child within the flow thread and
    // it is in the same inflow/out-of-flow layout context.
    if (renderer.IsOutOfFlowPositioned())
    {
      return nullptr;
    }

    krys_debug_assert(renderer.IsDescendantOf(&stayWithin));
    const RenderObject *current = &renderer;
    while (true)
    {
      // Skip to the first in-flow sibling.
      auto *nextSibling = current->NextSibling();
      while (nextSibling && nextSibling->IsOutOfFlowPositioned())
      {
        nextSibling = nextSibling->NextSibling();
      }
      if (nextSibling)
      {
        return nextSibling;
      }
      // No sibling candidate, jump to the parent and check its siblings.
      current = current->Parent();
      if (!current || current == &stayWithin || current->IsOutOfFlowPositioned())
      {
        return nullptr;
      }
    }
  }

  static bool IsValidColumnSpanner(const RenderMultiColumnFlow &fragmentedFlow,
                                   const RenderObject &descendant) noexcept
  {
    // We assume that we're inside the flow thread. This function is not to be called otherwise.
    krys_debug_assert(descendant.IsDescendantOf(&fragmentedFlow));
    // First make sure that the renderer itself has the right properties for becoming a spanner.
    auto *descendantBox = DynamicDowncast<RenderBox>(descendant);
    if (!descendantBox)
    {
      return false;
    }

    if (descendantBox->IsFloatingOrOutOfFlowPositioned())
    {
      return false;
    }

    if (!descendantBox->IsBlockLevelBox())
    {
      return false;
    }

    if (descendantBox->Style().GetColumnSpan() != ColumnSpan::All)
    {
      return false;
    }

    if (descendantBox->IsLegend())
    {
      return false;
    }

    if (!Is<RenderBlockFlow>(descendantBox->Parent()) && !Is<RenderInline>(descendantBox->Parent()))
    {
      return false;
    }

    // We need to have the flow thread as the containing block. A spanner cannot break out of the flow thread.
    if (descendantBox->EnclosingFragmentedFlow() != &fragmentedFlow)
    {
      return false;
    }

    // This looks like a spanner, but if we're inside something unbreakable, it's not to be treated as one.
    for (auto *ancestor = descendantBox->ContainingBlock(); ancestor; ancestor = ancestor->ContainingBlock())
    {
      if (Is<RenderView>(*ancestor))
      {
        return false;
      }
      if (ancestor->IsLegend())
      {
        return false;
      }
      if (Is<RenderTextControl>(*ancestor))
      {
        return false;
      }
      if (Is<RenderFragmentedFlow>(*ancestor))
      {
        // Don't allow any intervening non-multicol fragmentation contexts. The spec doesn't say
        // anything about disallowing this, but it's just going to be too complicated to
        // implement (not to mention specify behavior).
        return ancestor == &fragmentedFlow;
      }
      if (auto *blockFlowAncestor = DynamicDowncast<RenderBlockFlow>(*ancestor))
      {
        if (blockFlowAncestor->WillCreateColumns())
        {
          // This ancestor (descendent of the fragmentedFlow) will create columns later. The spanner belongs
          // to it.
          return false;
        }
        if (blockFlowAncestor->MultiColumnFlow())
        {
          // While this ancestor (descendent of the fragmentedFlow) has a fragmented flow context, this
          // context is being destroyed. However the spanner still belongs to it (will most likely be moved to
          // the parent fragmented context as the next step).
          return false;
        }
      }
      if (ancestor->IsUnsplittableForPagination())
      {
        return false;
      }
    }
    krys_debug_assert_msg(false, "A box in a multi-column flow has the flow as a containing block.");
    return false;
  }

  RenderTreeBuilder::MultiColumn::MultiColumn(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::MultiColumn::UpdateAfterDescendants(RenderBlockFlow &flow) noexcept
  {
    bool needsFragmentedFlow = flow.WillCreateColumns();
    bool hasFragmentedFlow = flow.MultiColumnFlow();

    if (!hasFragmentedFlow && needsFragmentedFlow)
    {
      CreateFragmentedFlow(flow);
      return;
    }
    if (hasFragmentedFlow && !needsFragmentedFlow)
    {
      DestroyFragmentedFlow(flow);
    }
  }

  RenderObject *
    RenderTreeBuilder::MultiColumn::ResolveMovedChild(RenderFragmentedFlow &enclosingFragmentedFlow,
                                                      RenderObject *beforeChild) noexcept
  {
    if (!beforeChild)
    {
      return nullptr;
    }

    auto *beforeChildRenderBox = DynamicDowncast<RenderBox>(*beforeChild);
    if (!beforeChildRenderBox)
    {
      return beforeChild;
    }

    auto *renderMultiColumnFlow = DynamicDowncast<RenderMultiColumnFlow>(enclosingFragmentedFlow);
    if (!renderMultiColumnFlow)
    {
      return beforeChild;
    }

    // We only need to resolve for column spanners.
    if (beforeChild->Style().GetColumnSpan() != ColumnSpan::All)
    {
      return beforeChild;
    }

    // The renderer for the actual DOM node that establishes a spanner is moved from its original
    // location in the render tree to becoming a sibling of the column sets. In other words, it's
    // moved out from the flow thread (and becomes a sibling of it). When we for instance want to
    // create and insert a renderer for the sibling node immediately preceding the spanner, we need
    // to map that spanner renderer to the spanner's placeholder, which is where the new inserted
    // renderer belongs.
    if (auto *placeholder = renderMultiColumnFlow->FindColumnSpannerPlaceholder(*beforeChildRenderBox))
    {
      return placeholder;
    }

    // This is an invalid spanner, or its placeholder hasn't been created yet. This happens when
    // moving an entire subtree into the flow thread, when we are processing the insertion of this
    // spanner's preceding sibling, and we obviously haven't got as far as processing this spanner
    // yet.
    return beforeChild;
  }

  void
    RenderTreeBuilder::MultiColumn::RestoreColumnSpannersForContainer(RenderMultiColumnFlow &multiColumnFlow,
                                                                      const RenderElement &container) noexcept
  {
    krys::List<RenderMultiColumnSpannerPlaceholder *> placeholdersToRestore;
    for (auto &[spanner, placeholder] : multiColumnFlow.GetSpannerMap())
    {
      if (placeholder && placeholder->IsDescendantOf(&container))
      {
        placeholdersToRestore.push_back(placeholder.get());
      }
    }
    for (auto *placeholder : placeholdersToRestore)
    {
      auto *spanner = placeholder->Spanner();
      if (!spanner)
      {
        krys_debug_assert_msg(false, "A placeholder keeps its spanner.");
        continue;
      }
      // Move the spanner back to its original position, in place of its placeholder. (WebKit lets detaching
      // the spanner destroy the placeholder, and appends the spanner to the placeholder's parent, out of
      // order.) Forgetting the placeholder first keeps it, to insert the spanner before, and still merges the
      // RenderMultiColumnSets around the spanner.
      multiColumnFlow.GetSpannerMap().erase(spanner);
      auto spannerToReInsert =
        _builder.Detach(*spanner->Parent(), *spanner, WillBeDestroyed::No, CanCollapseAnonymousBlock::No);
      {
        SetForScope restoringScope(s_restoringColumnSpannersForContainer, true);
        _builder.Attach(*placeholder->Parent(), krys::move(spannerToReInsert), placeholder);
      }
      _builder.Destroy(*placeholder);
    }
  }

  void RenderTreeBuilder::MultiColumn::MultiColumnDescendantInserted(RenderMultiColumnFlow &flow,
                                                                     RenderObject &newDescendant) noexcept
  {
    if (s_shiftingColumnSpanner || s_restoringColumnSpannersForContainer
        || newDescendant.IsRenderFragmentedFlow())
    {
      return;
    }

    auto *subtreeRoot = &newDescendant;
    auto *descendant = subtreeRoot;
    while (descendant)
    {
      // Skip nested multicolumn flows.
      if (Is<RenderMultiColumnFlow>(*descendant))
      {
        descendant = descendant->NextSibling();
        continue;
      }
      if (auto *placeholder = DynamicDowncast<RenderMultiColumnSpannerPlaceholder>(*descendant))
      {
        // A spanner's placeholder has been inserted. The actual spanner renderer is moved from
        // where it would otherwise occur (if it weren't a spanner) to becoming a sibling of the
        // column sets.
        krys_debug_assert(!flow.FindColumnSpannerPlaceholder(*placeholder->Spanner()));
        flow.GetSpannerMap().insert_or_assign(placeholder->Spanner(), CreateWeakPtr(placeholder));
        // There should be no children here, but if there are, we ought to skip them.
        krys_debug_assert(!placeholder->FirstChild());
      }
      else
      {
        descendant = ProcessPossibleSpannerDescendant(flow, subtreeRoot, *descendant);
      }
      if (descendant)
      {
        descendant = descendant->NextInPreOrder(subtreeRoot);
      }
    }
  }

  void RenderTreeBuilder::MultiColumn::MultiColumnRelativeWillBeRemoved(
    RenderMultiColumnFlow &flow, RenderObject &relative,
    CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept
  {
    flow.InvalidateFragments();
    if (auto *placeholder = DynamicDowncast<RenderMultiColumnSpannerPlaceholder>(relative))
    {
      // Remove the map entry for this spanner, but leave the actual spanner renderer alone. Also
      // keep the reference to the spanner, since the placeholder may be about to be re-inserted
      // in the tree.
      krys_debug_assert(relative.IsDescendantOf(&flow));
      flow.GetSpannerMap().erase(placeholder->Spanner());
      return;
    }
    if (relative.Style().GetColumnSpan() == ColumnSpan::All)
    {
      if (relative.Parent() != flow.Parent())
      {
        return; // not a valid spanner.
      }

      HandleSpannerRemoval(flow, relative, canCollapseAnonymousBlock);
    }
    // Note that we might end up with empty column sets if all column content is removed. That's no
    // big deal though (and locating them would be expensive), and they will be found and re-used if
    // content is added again later.
  }

  RenderObject *RenderTreeBuilder::MultiColumn::AdjustBeforeChildForMultiColumnSpannerIfNeeded(
    RenderObject &beforeChild) noexcept
  {
    auto *beforeChildBox = DynamicDowncast<RenderBox>(beforeChild);
    if (!beforeChildBox)
    {
      return &beforeChild;
    }

    auto *renderMultiColumnSet = DynamicDowncast<RenderMultiColumnSet>(beforeChildBox->NextSibling());
    if (!renderMultiColumnSet)
    {
      return &beforeChild;
    }

    auto *multiColumnFlow = renderMultiColumnSet->MultiColumnFlow();
    if (!multiColumnFlow)
    {
      return &beforeChild;
    }

    return multiColumnFlow->FindColumnSpannerPlaceholder(*beforeChildBox);
  }

  void RenderTreeBuilder::MultiColumn::CreateFragmentedFlow(RenderBlockFlow &flow) noexcept
  {
    // Do this to avoid wrapping inline children that are just going to move into the flow thread.
    flow.SetChildrenInline(false);
    flow.InvalidateLineLayout(RenderBlockFlow::InvalidationReason::InternalMove);
    // If this soon-to-be multicolumn flow is already part of a multicolumn context, we need to move back the
    // descendant spanners to their original position before moving subtrees around.
    if (auto *enclosingFlow = DynamicDowncast<RenderMultiColumnFlow>(flow.EnclosingFragmentedFlow()))
    {
      RestoreColumnSpannersForContainer(*enclosingFlow, flow);
    }

    auto newFragmentedFlow = CreateRenderer<RenderMultiColumnFlow>(
      flow.GetDocument(), RenderStyle::CreateAnonymousStyleWithDisplay(flow.Style(), DisplayType::Block));
    newFragmentedFlow->InitializeStyle();
    auto &fragmentedFlow = *newFragmentedFlow;
    _builder.BlockBuilder().Attach(flow, krys::move(newFragmentedFlow), nullptr);

    // Reparent children preceding the fragmented flow into the fragmented flow.
    _builder.MoveChildren(flow, fragmentedFlow, flow.FirstChild(), &fragmentedFlow,
                          NormalizeAfterInsertion::Yes);
    if (flow.IsFieldset())
    {
      // Keep legends out of the flow thread.
      krys::List<RenderBox *> legends;
      for (auto &box : ChildrenOfType<RenderBox>(fragmentedFlow))
      {
        if (box.IsLegend())
        {
          legends.push_back(&box);
        }
      }
      for (auto *legend : legends)
      {
        _builder.Move(fragmentedFlow, flow, *legend, NormalizeAfterInsertion::Yes);
      }
    }

    flow.SetMultiColumnFlow(fragmentedFlow);
  }

  void RenderTreeBuilder::MultiColumn::DestroyFragmentedFlow(RenderBlockFlow &flow) noexcept
  {
    auto &multiColumnFlow = *flow.MultiColumnFlow();
    multiColumnFlow.InvalidateLineLayout(RenderBlockFlow::InvalidationReason::InternalMove);

    // Move spanners back to their original DOM position in the tree (in the flow, which then moves out), and
    // destroy the placeholders.
    RestoreColumnSpannersForContainer(multiColumnFlow, multiColumnFlow);
    while (auto *columnSet = multiColumnFlow.FirstMultiColumnSet())
    {
      _builder.Destroy(*columnSet);
    }

    flow.ClearMultiColumnFlow();
    // We don't move the legend under the multicolumn flow (see CreateFragmentedFlow), so when the multicolumn
    // context is destroyed the fieldset already has a legend block level box.
    bool hasInitialBlockChild = false;
    if (flow.IsFieldset())
    {
      for (auto &box : ChildrenOfType<RenderBox>(flow))
      {
        if (box.IsLegend())
        {
          hasInitialBlockChild = true;
          break;
        }
      }
    }
    flow.SetChildrenInline(!hasInitialBlockChild);
    _builder.MoveAllChildren(multiColumnFlow, flow, NormalizeAfterInsertion::Yes);
    _builder.Destroy(multiColumnFlow);
  }

  RenderObject *RenderTreeBuilder::MultiColumn::ProcessPossibleSpannerDescendant(
    RenderMultiColumnFlow &flow, RenderObject *&subtreeRoot, RenderObject &descendant) noexcept
  {
    auto *multicolContainer = flow.MultiColumnBlockFlow();
    if (!multicolContainer)
    {
      return nullptr;
    }

    auto *nextRendererInFragmentedFlow = SpannerPlaceholderCandidate(descendant, flow);
    RenderObject *insertBeforeMulticolChild = nullptr;
    RenderObject *nextDescendant = &descendant;

    if (IsValidColumnSpanner(flow, descendant))
    {
      // This is a spanner (column-span:all). Such renderers are moved from where they would
      // otherwise occur in the render tree to becoming a direct child of the multicol container,
      // so that they live among the column sets. This simplifies the layout implementation, and
      // basically just relies on regular block layout done by the RenderBlockFlow that
      // establishes the multicol container.
      auto *container = descendant.Parent();
      if (nextRendererInFragmentedFlow)
      {
        if (auto *setToSplit = FindColumnSetRendering(flow, descendant))
        {
          setToSplit->SetNeedsLayout();
          insertBeforeMulticolChild = setToSplit->NextSibling();
        }
      }
      // Moving a spanner's renderer so that it becomes a sibling of the column sets requires us
      // to insert an anonymous placeholder in the tree where the spanner's renderer otherwise
      // would have been. This is needed for a two reasons: We need a way of separating inline
      // content before and after the spanner, so that it becomes separate line boxes. Secondly,
      // this placeholder serves as a break point for column sets, so that, when encountered, we
      // end flowing one column set and move to the next one.
      auto newPlaceholder = RenderMultiColumnSpannerPlaceholder::CreateAnonymous(
        flow, Downcast<RenderBox>(descendant), container->Style());
      auto &placeholder = *newPlaceholder;
      _builder.Attach(*container, krys::move(newPlaceholder), descendant.NextSibling());
      auto takenDescendant = _builder.Detach(*container, descendant, WillBeDestroyed::No);

      // This is a guard to stop an ancestor flow thread from processing the spanner.
      SetForScope shiftingScope(s_shiftingColumnSpanner, true);
      _builder.BlockBuilder().Attach(*multicolContainer, krys::move(takenDescendant),
                                     insertBeforeMulticolChild);

      // The spanner has now been moved out from the flow thread, but we don't want to
      // examine its children anyway. They are all part of the spanner and shouldn't trigger
      // creation of column sets or anything like that. Continue at its original position in
      // the tree, i.e. where the placeholder was just put.
      if (subtreeRoot == &descendant)
      {
        subtreeRoot = &placeholder;
      }
      nextDescendant = &placeholder;
    }
    else
    {
      // This is regular multicol content, i.e. not part of a spanner.
      if (auto *placeholder =
            DynamicDowncast<RenderMultiColumnSpannerPlaceholder>(nextRendererInFragmentedFlow))
      {
        // Inserted right before a spanner. Is there a set for us there?
        if (auto *previous = placeholder->Spanner()->PreviousSibling();
            previous && Is<RenderMultiColumnSet>(*previous))
        {
          return nextDescendant; // There's already a set there. Nothing to do.
        }
        insertBeforeMulticolChild = placeholder->Spanner();
      }
      else if (auto *lastSet = flow.LastMultiColumnSet())
      {
        // This child is not an immediate predecessor of a spanner, which means that if this
        // child precedes a spanner at all, there has to be a column set created for us there
        // already. If it doesn't precede any spanner at all, on the other hand, we need a
        // column set at the end of the multicol container. We don't really check here if the
        // child inserted precedes any spanner or not (as that's an expensive operation). Just
        // make sure we have a column set at the end. It's no big deal if it remains unused.

        // Legends are siblings of RenderMultiColumnSets not because they are spanners, but because they don't
        // participate in multi-column context.
        if (!lastSet->NextSibling() || lastSet->NextSibling()->IsLegend())
        {
          return nextDescendant;
        }
      }
    }
    // Need to create a new column set when there's no set already created. We also always insert
    // another column set after a spanner. Even if it turns out that there are no renderers
    // following the spanner, there may be bottom margins there, which take up space.
    auto newSet = CreateRenderer<RenderMultiColumnSet>(
      flow, RenderStyle::CreateAnonymousStyleWithDisplay(multicolContainer->Style(), DisplayType::Block));
    newSet->InitializeStyle();
    KRYS_MAYBE_UNUSED auto &set = *newSet;
    _builder.BlockBuilder().Attach(*multicolContainer, krys::move(newSet), insertBeforeMulticolChild);
    flow.InvalidateFragments();

    // We cannot handle immediate column set siblings at the moment (and there's no need for
    // it, either). There has to be at least one spanner separating them.
    krys_debug_assert(
      !RenderMultiColumnFlow::PreviousColumnSetOrSpannerSiblingOf(&set)
      || !RenderMultiColumnFlow::PreviousColumnSetOrSpannerSiblingOf(&set)->IsRenderMultiColumnSet());
    krys_debug_assert(
      !RenderMultiColumnFlow::NextColumnSetOrSpannerSiblingOf(&set)
      || !RenderMultiColumnFlow::NextColumnSetOrSpannerSiblingOf(&set)->IsRenderMultiColumnSet());

    return nextDescendant;
  }

  void RenderTreeBuilder::MultiColumn::HandleSpannerRemoval(
    RenderMultiColumnFlow &flow, RenderObject &spanner,
    CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept
  {
    // The placeholder may already have been removed, but if it hasn't, do so now.
    auto &spannerMap = flow.GetSpannerMap();
    if (auto node = spannerMap.extract(&Downcast<RenderBox>(spanner)); node && node.mapped())
    {
      _builder.Destroy(*node.mapped(), canCollapseAnonymousBlock);
    }

    if (auto *next = spanner.NextSibling())
    {
      if (auto *previous = spanner.PreviousSibling())
      {
        if (previous->IsRenderMultiColumnSet() && next->IsRenderMultiColumnSet())
        {
          // Merge two sets that no longer will be separated by a spanner.
          _builder.Destroy(*next);
          previous->SetNeedsLayout();
        }
      }
    }
  }
}
