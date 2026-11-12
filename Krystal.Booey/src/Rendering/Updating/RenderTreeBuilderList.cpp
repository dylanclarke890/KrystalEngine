/**
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Andrew Wellington (proton@wiretapped.net)
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
 *
 */

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderList.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/GeneratedContent/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/Iterators/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/MultiColumn/RenderMultiColumnFlow.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Rendering/Tables/RenderTable.hpp"
#include "Krystal.Booey/Style/Values/Lists/ListStyleType.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD Pair<RenderBlock *, RenderBlock *>
      FindParentOfEmptyOrFirstLineBox(RenderBlock &blockContainer, const RenderListMarker &marker) noexcept
    {
      auto inQuirksMode = blockContainer.GetDocument().InQuirksMode();
      RenderBlock *fallbackParent = {};

      for (auto &child : ChildrenOfType<RenderObject>(blockContainer))
      {
        if (&child == &marker)
        {
          continue;
        }

        if (child.IsInline())
        {
          if (!Is<RenderInline>(child) || !IsEmptyInline(Downcast<RenderInline>(child)))
          {
            return {&blockContainer, {}};
          }
          fallbackParent = &blockContainer;
        }

        if (child.IsFloating() || child.IsOutOfFlowPositioned() || child.IsRenderMenuList())
        {
          continue;
        }

        if (auto *renderBox = DynamicDowncast<RenderBox>(child); renderBox && renderBox->IsWritingModeRoot())
        {
          break;
        }

        if (Is<RenderListItem>(blockContainer) && inQuirksMode && child.GetNode()
            && IsHTMLListElement(*child.GetNode()))
        {
          break;
        }

        if (!Is<RenderBlock>(child) || Is<RenderTable>(child))
        {
          break;
        }

        auto &blockChild = Downcast<RenderBlock>(child);
        auto [nestedParent, nestedFallbackParent] = FindParentOfEmptyOrFirstLineBox(blockChild, marker);
        if (nestedParent)
        {
          return {nestedParent, {}};
        }

        if (!fallbackParent)
        {
          if (nestedFallbackParent)
          {
            fallbackParent = nestedFallbackParent;
          }
          else if (auto *firstInFlowChild = blockChild.FirstInFlowChild();
                   !firstInFlowChild || firstInFlowChild == &marker)
          {
            fallbackParent = &blockChild;
          }
        }
      }

      return {{}, fallbackParent};
    }

    KRYS_NODISCARD RenderBlock *ParentCandidateForMarker(RenderListItem &listItemRenderer,
                                                         const RenderListMarker &marker) noexcept
    {
      if (marker.IsInside())
      {
        if (auto *firstChild = DynamicDowncast<RenderBlock>(listItemRenderer.FirstChild()))
        {
          if (!firstChild->IsAnonymous())
          {
            return &listItemRenderer;
          }
          // We may have created this anonymous block for the marker itself. Let's keep it in there.
          if (firstChild->FirstChild() == &marker && !marker.NextSibling())
          {
            return firstChild;
          }
        }
        return FindParentOfEmptyOrFirstLineBox(listItemRenderer, marker).first;
      }
      auto [parentCandidate, fallbackParent] = FindParentOfEmptyOrFirstLineBox(listItemRenderer, marker);
      return parentCandidate ? parentCandidate : fallbackParent;
    }

    KRYS_NODISCARD RenderObject *FirstNonMarkerChild(RenderBlock &parent) noexcept
    {
      RenderObject *child = parent.FirstChild();
      while (Is<RenderListMarker>(child))
      {
        child = child->NextSibling();
      }
      return child;
    }
  }

  RenderTreeBuilder::List::List(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::List::UpdateItemMarker(RenderListItem &listItemRenderer) noexcept
  {
    const auto &style = listItemRenderer.Style();

    if (auto *element = DynamicDowncast<html::HTMLElement>(listItemRenderer.GetElement());
        element && Is<html::HTMLFieldSetElement>(*element))
    {
      if (auto *marker = listItemRenderer.MarkerRenderer())
      {
        _builder.Destroy(*marker);
      }
      return;
    }

    if (auto styleImage = style.ListStyleImage().TryStyleImage();
        style.GetListStyleType().IsNone() && (!styleImage || styleImage->ErrorOccurred()))
    {
      if (auto *marker = listItemRenderer.MarkerRenderer())
      {
        _builder.Destroy(*marker);
      }
      return;
    }

    auto newStyle = listItemRenderer.ComputeMarkerStyle();
    if (auto *markerRenderer = listItemRenderer.MarkerRenderer())
    {
      markerRenderer->SetStyle(krys::move(newStyle));
      auto *currentParent = markerRenderer->Parent();
      if (!currentParent)
      {
        krys_debug_assert(false);
        return;
      }

      RenderBlock *newParent = ParentCandidateForMarker(listItemRenderer, *markerRenderer);
      if (!newParent)
      {
        if (currentParent->IsAnonymousBlock())
        {
          // If the marker is currently contained inside an anonymous box. then we are the only item in that
          // anonymous box (since no line box parent was found). It's ok to just leave the marker where it is
          // in this case.
          return;
        }
        newParent = &listItemRenderer;
        if (auto *multiColumnFlow = listItemRenderer.MultiColumnFlow())
        {
          newParent = multiColumnFlow;
        }
      }

      if (newParent == currentParent)
      {
        return;
      }

      _builder.Attach(
        *newParent,
        _builder.Detach(*currentParent, *markerRenderer, WillBeDestroyed::No, CanCollapseAnonymousBlock::No),
        FirstNonMarkerChild(*newParent));
      // If current parent is an anonymous block that has lost all its children, destroy it.
      if (currentParent->IsAnonymousBlock() && !currentParent->FirstChild())
      {
        _builder.Destroy(*currentParent);
      }
      return;
    }

    auto newMarkerRenderer = CreateRenderer<RenderListMarker>(listItemRenderer, krys::move(newStyle));
    newMarkerRenderer->InitializeStyle();
    listItemRenderer.SetMarkerRenderer(*newMarkerRenderer);
    RenderBlock *newParent = ParentCandidateForMarker(listItemRenderer, *newMarkerRenderer);
    if (!newParent)
    {
      // If the marker is currently contained inside an anonymous box,
      // then we are the only item in that anonymous box (since no line box
      // parent was found). It's ok to just leave the marker where it is
      // in this case.
      newParent = &listItemRenderer;
      if (auto *multiColumnFlow = listItemRenderer.MultiColumnFlow())
      {
        newParent = multiColumnFlow;
      }
    }
    _builder.Attach(*newParent, krys::move(newMarkerRenderer), FirstNonMarkerChild(*newParent));
  }
}
