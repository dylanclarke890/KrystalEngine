/*
 * Copyright (C) 2006-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Alexander Kellett <lypanov@kde.org>
 * Copyright (C) 2006 Oliver Hunt <ojh16@student.canterbury.ac.nz>
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
 * Copyright (C) 2009 Dirk Schulze <krit@webkit.org>
 * Copyright (C) Research In Motion Limited 2010-2012. All rights reserved.
 * Copyright (C) 2012-2023 Google Inc.
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

#include "Krystal.Web/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Core/Geometry/Quad.hpp"
#include "Krystal.Core/Strings/CodePointIterator.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Editing/Position.hpp"
#include "Krystal.Web/Graphics/GraphicsContextStateSaver.hpp"
#include "Krystal.Web/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Web/Layout/Integration/Inline/InlineIteratorLogicalOrderTraversal.hpp"
#include "Krystal.Web/Layout/Integration/Inline/InlineIteratorSVGTextBox.hpp"
#include "Krystal.Web/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Web/Rendering/HitTesting/HitTestLocation.hpp"
#include "Krystal.Web/Rendering/HitTesting/HitTestRequest.hpp"
#include "Krystal.Web/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Web/Rendering/HitTesting/PointerEventsHitRules.hpp"
#include "Krystal.Web/Rendering/Iterators/RenderAncestorIterator.hpp"
#include "Krystal.Web/Rendering/LayoutRepainter.hpp"
#include "Krystal.Web/Rendering/Painting/PaintInfo.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Web/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Web/Rendering/SVG/RenderSVGInline.hpp"
#include "Krystal.Web/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Web/Rendering/SVG/RenderSVGTextPath.hpp"
#include "Krystal.Web/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Web/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Web/Rendering/SVG/SVGTextBoxPainter.hpp"
#include "Krystal.Web/Rendering/SVG/SVGTextLayoutEngine.hpp"
#include "Krystal.Web/Rendering/SVG/SVGVisitedRendererTracking.hpp"
#include "Krystal.Web/Style/Values/Borders/Shadow.hpp"
#include "Krystal.Web/Svg/Elements/SVGTextElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGTextPathElement.hpp"
#include "Krystal.Web/Svg/SVGLengthContext.hpp"
#include <algorithm>
#include <tuple>

namespace krys::web
{
  RenderSVGText::RenderSVGText(SVGTextElement &element, RenderStyle &&style) noexcept
      : RenderSVGBlock(Type::SVGText, element, krys::move(style))
  {
    krys_debug_assert(IsRenderSVGText());
  }

  RenderSVGText::~RenderSVGText() noexcept
  {
    krys_debug_assert(_layoutAttributes.empty());
  }

  SVGTextElement &RenderSVGText::TextElement() const noexcept
  {
    return Downcast<SVGTextElement>(RenderSVGBlock::GraphicsElement());
  }

  bool RenderSVGText::IsChildAllowed(const RenderObject &child, const RenderStyle &) const noexcept
  {
    auto isEmptySVGInlineText = [](const RenderObject *object)
    {
      const auto *svgInlineText = DynamicDowncast<RenderSVGInlineText>(object);
      return svgInlineText && svgInlineText->HasEmptyText();
    };

    return child.IsInline() && !isEmptySVGInlineText(&child);
  }

  RenderSVGText *RenderSVGText::LocateRenderSVGTextAncestor(RenderObject &start) noexcept
  {
    return LineageOfType<RenderSVGText>(start).First();
  }

  const RenderSVGText *RenderSVGText::LocateRenderSVGTextAncestor(const RenderObject &start) noexcept
  {
    return LineageOfType<RenderSVGText>(start).First();
  }

  namespace
  {
    void CollectLayoutAttributes(RenderObject *text, List<SVGTextLayoutAttributes *> &attributes) noexcept
    {
      for (RenderObject *descendant = text; descendant; descendant = descendant->NextInPreOrder(text))
      {
        if (auto *svgInline = DynamicDowncast<RenderSVGInlineText>(*descendant))
        {
          attributes.push_back(svgInline->LayoutAttributes());
        }
      }
    }

    KRYS_NODISCARD std::tuple<SVGTextLayoutAttributes *, SVGTextLayoutAttributes *>
      FindPreviousAndNextAttributes(RenderElement &root, RenderSVGInlineText &locateElement) noexcept
    {
      SVGTextLayoutAttributes *previous = nullptr;
      bool foundLocateElement = false;

      for (CheckedPtr<RenderObject> current = ShareCheckedPtr(root.FirstChild()); current;)
      {
        if (auto *childSVGInline = DynamicDowncast<RenderSVGInline>(*current))
        {
          if (auto *child = childSVGInline->FirstChild())
          {
            current = ShareCheckedPtr(child);
          }
          else
          {
            current = ShareCheckedPtr(current->NextInPreOrderAfterChildren(&root));
          }
          continue;
        }

        if (auto *text = DynamicDowncast<RenderSVGInlineText>(*current))
        {
          if (foundLocateElement)
          {
            return {previous, text->LayoutAttributes()};
          }

          if (&locateElement == text)
          {
            foundLocateElement = true;
          }
          else
          {
            previous = text->LayoutAttributes();
          }
        }

        current = ShareCheckedPtr(current->NextInPreOrderAfterChildren(&root));
      }

      return {previous, nullptr};
    }

    void CheckLayoutAttributesConsistency(
      KRYS_MAYBE_UNUSED RenderSVGText *text,
      KRYS_MAYBE_UNUSED List<SVGTextLayoutAttributes *> &expectedLayoutAttributes) noexcept
    {
#ifndef NDEBUG
      List<SVGTextLayoutAttributes *> newLayoutAttributes;
      CollectLayoutAttributes(text, newLayoutAttributes);
      krys_debug_assert(newLayoutAttributes == expectedLayoutAttributes);
#endif
    }

    void UpdateFontInAllDescendants(RenderSVGText &text) noexcept
    {
      for (RenderObject *descendant = &text; descendant; descendant = descendant->NextInPreOrder(&text))
      {
        if (auto *inlineText = DynamicDowncast<RenderSVGInlineText>(*descendant))
        {
          inlineText->UpdateScaledFont();
        }
      }
    }
  }

  bool RenderSVGText::ShouldHandleSubtreeMutations() const noexcept
  {
    if (BeingDestroyed() || !_hasPerformedLayout)
    {
      krys_debug_assert(_layoutAttributes.empty());
      krys_debug_assert(!_layoutAttributesBuilder.NumberOfTextPositioningElements());
      return false;
    }
    return true;
  }

  void RenderSVGText::SubtreeChildWasAdded(RenderObject *child) noexcept
  {
    krys_debug_assert(child);
    if (!ShouldHandleSubtreeMutations() || RenderTreeBeingDestroyed())
    {
      return;
    }

    // The positioning elements cache doesn't include the new 'child' yet. Clear the
    // cache, as the next BuildLayoutAttributesForTextRenderer() call rebuilds it.
    _layoutAttributesBuilder.ClearTextPositioningElements();

    if (!child->IsRenderSVGInlineText() && !child->IsRenderSVGInline())
    {
      return;
    }

    // Detect changes in layout attributes and only measure those text parts that have changed!
    List<SVGTextLayoutAttributes *> newLayoutAttributes;
    CollectLayoutAttributes(this, newLayoutAttributes);
    if (newLayoutAttributes.empty())
    {
      krys_debug_assert(_layoutAttributes.empty());
      return;
    }

    // Compare _layoutAttributes with newLayoutAttributes to figure out which attribute got added.
    SVGTextLayoutAttributes *attributes = nullptr;
    for (auto *newAttributes : newLayoutAttributes)
    {
      attributes = newAttributes;
      if (std::ranges::find(_layoutAttributes, attributes) == _layoutAttributes.end())
      {
        // Every time this is invoked, there's only a single new entry in the newLayoutAttributes list,
        // compared to the old in _layoutAttributes.
        krys_debug_assert(&attributes->Context() == child);
        auto [previous, next] = FindPreviousAndNextAttributes(*this, attributes->Context());

        if (previous)
        {
          _layoutAttributesBuilder.BuildLayoutAttributesForTextRenderer(previous->Context());
        }
        _layoutAttributesBuilder.BuildLayoutAttributesForTextRenderer(attributes->Context());
        if (next)
        {
          _layoutAttributesBuilder.BuildLayoutAttributesForTextRenderer(next->Context());
        }
        break;
      }
    }

#ifndef NDEBUG
    // Verify that _layoutAttributes only differs by a maximum of one entry.
    for (auto *newAttributes : newLayoutAttributes)
    {
      krys_debug_assert(std::ranges::find(_layoutAttributes, newAttributes) != _layoutAttributes.end()
                        || newAttributes == attributes);
    }
#endif

    _layoutAttributes = krys::move(newLayoutAttributes);
  }

  void RenderSVGText::WillBeDestroyed() noexcept
  {
    _layoutAttributes.clear();
    _layoutAttributesBuilder.ClearTextPositioningElements();

    RenderSVGBlock::WillBeDestroyed();
  }

  void RenderSVGText::SubtreeChildWillBeRemoved(
    RenderObject *child, SmallList<SVGTextLayoutAttributes *, 2> &affectedAttributes) noexcept
  {
    krys_debug_assert(child);
    if (!ShouldHandleSubtreeMutations())
    {
      return;
    }

    CheckLayoutAttributesConsistency(this, _layoutAttributes);

    // The positioning elements cache depends on the size of each text renderer in the
    // subtree. If this changes, clear the cache. It's going to be rebuilt below.
    _layoutAttributesBuilder.ClearTextPositioningElements();
    if (_layoutAttributes.empty() || !child->IsRenderSVGInlineText())
    {
      return;
    }

    // This logic requires that the 'text' child is still inserted in the tree.
    auto &text = Downcast<RenderSVGInlineText>(*child);
    if (!RenderTreeBeingDestroyed())
    {
      auto [previous, next] = FindPreviousAndNextAttributes(*this, text);
      if (previous)
      {
        affectedAttributes.push_back(previous);
      }
      if (next)
      {
        affectedAttributes.push_back(next);
      }
    }

    auto it = std::ranges::find(_layoutAttributes, text.LayoutAttributes());
    krys_debug_assert(it != _layoutAttributes.end());
    if (it != _layoutAttributes.end())
    {
      _layoutAttributes.erase(it);
    }
  }

  void RenderSVGText::SubtreeChildWasRemoved(
    const SmallList<SVGTextLayoutAttributes *, 2> &affectedAttributes) noexcept
  {
    if (!ShouldHandleSubtreeMutations() || RenderTreeBeingDestroyed())
    {
      krys_debug_assert(affectedAttributes.empty());
      return;
    }

    // This is called immediately after SubtreeChildWillBeRemoved, once the
    // RenderSVGInlineText::WillBeDestroyed() method passes on to the base class, which removes us from the
    // render tree. At this point we can update the layout attributes.
    for (auto *attributes : affectedAttributes)
    {
      _layoutAttributesBuilder.BuildLayoutAttributesForTextRenderer(attributes->Context());
    }
  }

  void RenderSVGText::SubtreeTextDidChange(RenderSVGInlineText *text) noexcept
  {
    krys_debug_assert(text);
    krys_debug_assert(!BeingDestroyed());
    if (!_hasPerformedLayout)
    {
      krys_debug_assert(_layoutAttributes.empty());
      krys_debug_assert(!_layoutAttributesBuilder.NumberOfTextPositioningElements());
      return;
    }
    // Text transforms can cause text change to be signaled during addChild before _layoutAttributes has been
    // updated.
    if (std::ranges::find(_layoutAttributes, text->LayoutAttributes()) == _layoutAttributes.end())
    {
      krys_debug_assert(!text->EverHadLayout());
      return;
    }

    // The positioning elements cache depends on the size of each text renderer in the
    // subtree. If this changes, clear the cache and mark it for rebuilding in the next layout.
    _layoutAttributesBuilder.ClearTextPositioningElements();

    CheckLayoutAttributesConsistency(this, _layoutAttributes);
    SetNeedsPositioningValuesUpdate();
    SetNeedsLayout();
  }

  void RenderSVGText::Layout() noexcept
  {
    krys_debug_assert(NeedsLayout());

    if (ShouldHandleSubtreeMutations() && !RenderTreeBeingDestroyed())
    {
      CheckLayoutAttributesConsistency(this, _layoutAttributes);
    }

    LayoutRepainter repainter(*this, SVGRenderSupport::CheckForSVGRepaintDuringLayout(*this));

    bool updateCachedBoundariesInParents = false;
    auto previousReferenceBoxRect = TransformReferenceBoxRect();

    // We update the transform now because UpdateScaledFont() needs it, but we do it a second time at the end
    // of the layout, since the transform reference box may change because of the font change.
    if (_needsTransformUpdate)
    {
      _localTransform = TextElement().AnimatedLocalTransform();
      updateCachedBoundariesInParents = true;
    }

    if (!_hasPerformedLayout)
    {
      // When laying out initially, collect all layout attributes, build the character data map,
      // and propogate resulting SVGLayoutAttributes to all RenderSVGInlineText children in the subtree.
      krys_debug_assert(_layoutAttributes.empty());
      CollectLayoutAttributes(this, _layoutAttributes);
      UpdateFontInAllDescendants(*this);
      _layoutAttributesBuilder.BuildLayoutAttributesForForSubtree(*this);

      _needsReordering = true;
      _needsTextMetricsUpdate = false;
      _needsPositioningValuesUpdate = false;
      updateCachedBoundariesInParents = true;
    }
    else if (_needsPositioningValuesUpdate)
    {
      // When the x/y/dx/dy/rotate lists change, recompute the layout attributes, and eventually
      // update the on-screen font objects as well in all descendants.
      if (_needsTextMetricsUpdate)
      {
        UpdateFontInAllDescendants(*this);
      }
      _layoutAttributesBuilder.BuildLayoutAttributesForForSubtree(*this);
      _needsReordering = true;
      _needsTextMetricsUpdate = false;
      _needsPositioningValuesUpdate = false;
      updateCachedBoundariesInParents = true;
    }
    else
    {
      bool isLayoutSizeChanged = false;
      if (auto *legacyRootObject = LineageOfType<LegacyRenderSVGRoot>(*this).First())
      {
        isLayoutSizeChanged = legacyRootObject->IsLayoutSizeChanged();
      }

      if (_needsTextMetricsUpdate || isLayoutSizeChanged || _needsTransformUpdate)
      {
        // If the root layout size changed (eg. window size changes) or the transform to the root
        // context has changed then recompute the on-screen font size.
        UpdateFontInAllDescendants(*this);

        krys_debug_assert(!_needsReordering);
        krys_debug_assert(!_needsPositioningValuesUpdate);
        _needsTextMetricsUpdate = false;
        updateCachedBoundariesInParents = true;
      }
      _layoutAttributesBuilder.RebuildMetricsForSubtree(*this);
    }

    CheckLayoutAttributesConsistency(this, _layoutAttributes);

    // Reduced version of RenderBlock::LayoutBlock(), which only takes care of SVG text.
    // All if branches that could cause early exit in RenderBlocks LayoutBlock() method are turned into
    // assertions.
    krys_debug_assert(!IsInline());
    krys_debug_assert(!IsAnonymousBlock());

    // FIXME: We need to find a way to only layout the child boxes, if needed.
    auto layoutChanged = _hasPerformedLayout && SelfNeedsLayout();
    auto oldBoundaries = ObjectBoundingBox();

    if (!FirstChild())
    {
      UpdatePositionAndOverflow({});
      SetChildrenInline(true);
    }

    krys_debug_assert(ChildrenInline());

    LayoutUnit repaintLogicalTop;
    LayoutUnit repaintLogicalBottom;
    RebuildFloatingObjectSetFromIntrudingFloats();
    LayoutInlineChildren(RelayoutChildren::Yes, LogicalHeight(), repaintLogicalTop, repaintLogicalBottom);

    ComputePerCharacterLayoutInformation();

    // UpdatePositionAndOverflow() is called by LayoutRootBox (or with the inline content's boundaries),
    // before this point is reached.
    if (_needsReordering)
    {
      _needsReordering = false;
    }

    if (_needsTransformUpdate)
    {
      if (previousReferenceBoxRect != TransformReferenceBoxRect())
      {
        _localTransform = TextElement().AnimatedLocalTransform();
      }
      _needsTransformUpdate = false;
    }
    if (!updateCachedBoundariesInParents)
    {
      updateCachedBoundariesInParents = oldBoundaries != ObjectBoundingBox();
    }

    // Invalidate all resources of this client if our layout changed.
    if (layoutChanged)
    {
      SVGResourcesCache::ClientLayoutChanged(*this);
    }

    // If our bounds changed, notify the parents.
    if (updateCachedBoundariesInParents)
    {
      if (auto *parent = Parent())
      {
        parent->InvalidateCachedBoundaries();
      }
    }

    repainter.RepaintAfterLayout();
    ClearNeedsLayout();
    _hasPerformedLayout = true;
  }

  void RenderSVGText::ComputePerCharacterLayoutInformation() noexcept
  {
    if (!IsObjectBoundingBoxValid())
    {
      return;
    }

    if (_layoutAttributes.empty())
    {
      return;
    }

    if (_needsReordering)
    {
      ReorderValueListsToLogicalOrder();
    }

    // Perform SVG text layout phase two (see SVGTextLayoutEngine for details).
    SVGTextLayoutEngine characterLayout(_layoutAttributes);

    LayoutCharactersInTextBoxes(inline_iterator::FirstRootInlineBoxFor(*this), characterLayout);

    // Perform SVG text layout phase three (see SVGTextChunkBuilder for details).
    auto fragmentMap = characterLayout.FinishLayout();

    // Perform SVG text layout phase four: position the boxes from their fragments.
    auto boundaries = InlineLayout()->ApplySVGTextFragments(krys::move(fragmentMap));
    UpdatePositionAndOverflow(boundaries);
  }

  void RenderSVGText::LayoutCharactersInTextBoxes(const inline_iterator::InlineBoxIterator &parent,
                                                  SVGTextLayoutEngine &characterLayout) noexcept
  {
    auto [begin, end] = parent->Descendants();

    for (auto child = begin; child != end; child.TraverseLineRightwardOnLineSkippingChildren())
    {
      if (child->IsSVGText())
      {
        characterLayout.LayoutInlineTextBox(inline_iterator::SVGTextBoxIterator(*child));
        continue;
      }

      // Skip generated content.
      RefPtr node = ShareRefPtr(child->Renderer().GetNode());
      if (!node)
      {
        continue;
      }

      auto *inlineBox = DynamicDowncast<inline_iterator::InlineBox>(*child);
      if (!inlineBox)
      {
        continue;
      }

      auto *svgElement = DynamicDowncast<SVGElement>(*node);
      bool isTextPath = svgElement && Is<SVGTextPathElement>(*svgElement);
      if (isTextPath)
      {
        // Build text chunks for all <textPath> children, using the line layout algorithm.
        // This is needeed as text-anchor is just an additional startOffset for text paths.
        SVGTextLayoutEngine lineLayout(characterLayout.LayoutAttributes());
        LayoutCharactersInTextBoxes(inlineBox->Iterator(), lineLayout);

        characterLayout.BeginTextPathLayout(Downcast<RenderSVGTextPath>(child->Renderer()), lineLayout);
      }

      LayoutCharactersInTextBoxes(inlineBox->Iterator(), characterLayout);

      if (isTextPath)
      {
        characterLayout.EndTextPathLayout();
      }
    }
  }

  namespace
  {
    void SwapItemsInLayoutAttributes(SVGTextLayoutAttributes *firstAttributes,
                                     SVGTextLayoutAttributes *lastAttributes, uint32 firstPosition,
                                     uint32 lastPosition) noexcept
    {
      auto itFirst = firstAttributes->CharacterDataMap().find(firstPosition + 1);
      auto itLast = lastAttributes->CharacterDataMap().find(lastPosition + 1);
      bool firstPresent = itFirst != firstAttributes->CharacterDataMap().end();
      bool lastPresent = itLast != lastAttributes->CharacterDataMap().end();
      // We only want to perform the swap if both inline boxes are absolutely positioned.
      if (!firstPresent || !lastPresent)
      {
        return;
      }

      std::swap(itFirst->second, itLast->second);
    }

    void FindFirstAndLastAttributesInVector(List<SVGTextLayoutAttributes *> &attributes,
                                            const RenderSVGInlineText *firstContext,
                                            const RenderSVGInlineText *lastContext,
                                            SVGTextLayoutAttributes *&first,
                                            SVGTextLayoutAttributes *&last) noexcept
    {
      first = nullptr;
      last = nullptr;

      for (auto *current : attributes)
      {
        if (!first && firstContext == &current->Context())
        {
          first = current;
        }
        if (!last && lastContext == &current->Context())
        {
          last = current;
        }
        if (first && last)
        {
          break;
        }
      }

      krys_debug_assert(first);
      krys_debug_assert(last);
    }

    KRYS_NODISCARD bool IsSingleCharacter(const inline_iterator::SVGTextBox &textBox) noexcept
    {
      auto text = utf8_stringview(textBox.Renderer().Text()).substr(textBox.Start(), textBox.Length());
      return !text.empty()
             && krys::text::DecodeUTF8(Span<const char8>(text.data(), text.size())).Length == text.size();
    }

    void ReverseInlineBoxRangeAndValueListsIfNeeded(List<SVGTextLayoutAttributes *> &attributes,
                                                    std::span<inline_iterator::LeafBoxIterator> span) noexcept
    {
      // This is a copy of std::reverse(first, last). It additionally assures that the metrics map within the
      // renderers belonging to the inline boxes are reordered as well.
      while (true)
      {
        if (span.size() <= 1)
        {
          return;
        }
        auto *firstTextBox = DynamicDowncast<inline_iterator::SVGTextBox>(*span.front());
        auto *lastTextBox = DynamicDowncast<inline_iterator::SVGTextBox>(*span.back());

        // Reordering is only necessary for bidi text that is _absolutely_ positioned.
        if (firstTextBox && lastTextBox && IsSingleCharacter(*firstTextBox)
            && IsSingleCharacter(*lastTextBox))
        {
          const auto &firstContext = firstTextBox->Renderer();
          const auto &lastContext = lastTextBox->Renderer();

          SVGTextLayoutAttributes *firstAttributes = nullptr;
          SVGTextLayoutAttributes *lastAttributes = nullptr;
          FindFirstAndLastAttributesInVector(attributes, &firstContext, &lastContext, firstAttributes,
                                             lastAttributes);
          SwapItemsInLayoutAttributes(firstAttributes, lastAttributes, firstTextBox->Start(),
                                      lastTextBox->Start());
        }

        std::swap(span.front(), span.back());

        span = span.subspan(1, span.size() - 2);
      }
    }
  }

  void RenderSVGText::ReorderValueListsToLogicalOrder() noexcept
  {
    for (auto lineBox = inline_iterator::FirstLineBoxFor(*this); lineBox; lineBox.TraverseNext())
    {
      std::ignore = inline_iterator::LeafBoxesInLogicalOrder(
        lineBox, [&](auto span) { ReverseInlineBoxRangeAndValueListsIfNeeded(_layoutAttributes, span); });
    }
  }

  bool RenderSVGText::NodeAtPointF(const HitTestRequest &request, HitTestResult &result,
                                   const PointF &pointInParent, HitTestAction hitTestAction) noexcept
  {
    PointerEventsHitRules hitRules(PointerEventsHitRules::HitTestingTargetType::SVGText, request,
                                   Style().UsedPointerEvents());
    if (IsVisibleToHitTesting(Style(), request) || !hitRules.RequireVisible)
    {
      if ((hitRules.CanHitStroke && (Style().HasStroke() || !hitRules.RequireStroke))
          || (hitRules.CanHitFill && (Style().HasFill() || !hitRules.RequireFill)))
      {
        static SVGVisitedRendererTracking::VisitedSet visitedSet;

        SVGVisitedRendererTracking recursionTracking(visitedSet);
        if (recursionTracking.IsVisiting(*this))
        {
          return false;
        }

        SVGVisitedRendererTracking::Scope recursionScope(recursionTracking, *this);

        PointF localPoint =
          LocalToParentTransform().Inverse().value_or(AffineTransform {}).MapPoint(pointInParent);
        if (!SVGRenderSupport::PointInClippingArea(*this, localPoint))
        {
          return false;
        }

        HitTestLocation hitTestLocation(LayoutPoint(FlooredPointI(localPoint)));
        return RenderBlock::NodeAtPoint(request, result, hitTestLocation, LayoutPoint(), hitTestAction);
      }
    }

    return false;
  }

  bool RenderSVGText::HitTestInlineChildren(const HitTestRequest &request, HitTestResult &result,
                                            const HitTestLocation &locationInContainer,
                                            const LayoutPoint &accumulatedOffset, HitTestAction) noexcept
  {
    auto hitTestInlineBoxes = [&]
    {
      for (auto &box : inline_iterator::BoxesFor(*this))
      {
        auto *textBox = DynamicDowncast<inline_iterator::SVGTextBox>(box);
        if (!textBox)
        {
          continue;
        }

        PointerEventsHitRules hitRules(PointerEventsHitRules::HitTestingTargetType::SVGText, request,
                                       Style().UsedPointerEvents());

        auto &renderer = textBox->Renderer();
        if (!IsVisibleToHitTesting(renderer.Style(), request) && hitRules.RequireVisible)
        {
          continue;
        }

        bool hitsStroke = hitRules.CanHitStroke && (renderer.Style().HasStroke() || !hitRules.RequireStroke);
        bool hitsFill = hitRules.CanHitFill && (renderer.Style().HasFill() || !hitRules.RequireFill);
        if (!hitsStroke && !hitsFill)
        {
          continue;
        }

        RectF rect = textBox->LogicalRectIgnoringInlineDirection();
        rect.MoveBy(accumulatedOffset);
        if (!locationInContainer.Intersects(rect))
        {
          continue;
        }

        float scalingFactor = renderer.ScalingFactor();
        krys_debug_assert(scalingFactor);

        float baseline = renderer.ScaledFont().MetricsOfPrimaryFont().Ascent() / scalingFactor;

        AffineTransform fragmentTransform;
        for (auto &fragment : textBox->TextFragments())
        {
          QuadF fragmentQuad(RectF(fragment.X, fragment.Y - baseline, fragment.Width, fragment.Height));
          fragment.BuildFragmentTransform(fragmentTransform);
          if (!fragmentTransform.IsIdentity())
          {
            fragmentQuad = fragmentTransform.MapQuad(fragmentQuad);
          }

          if (!fragmentQuad.ContainsPoint(locationInContainer.Point()))
          {
            continue;
          }

          renderer.UpdateHitTestResult(result, locationInContainer.Point() - ToLayoutSize(accumulatedOffset));
          if (result.AddNodeToListBasedTestResult(renderer.NodeForHitTest(), request, locationInContainer,
                                                  rect)
              == HitTestProgress::Stop)
          {
            return true;
          }
        }
      }
      return false;
    };

    if (hitTestInlineBoxes())
    {
      UpdateHitTestResult(result, locationInContainer.Point() - ToLayoutSize(accumulatedOffset));
      return true;
    }

    return false;
  }

  PositionWithAffinity RenderSVGText::PositionForPoint(const LayoutPoint &pointInContents,
                                                       HitTestSource source,
                                                       const RenderFragmentContainer * /*fragment*/) noexcept
  {
    inline_iterator::BoxIterator closestBox;
    inline_iterator::BoxIterator lastBox;
    for (auto &box : inline_iterator::BoxesFor(*this))
    {
      auto *textBox = DynamicDowncast<inline_iterator::SVGTextBox>(box);
      if (!textBox)
      {
        continue;
      }
      lastBox = inline_iterator::BoxIterator {*textBox};
      auto rect = textBox->VisualRectIgnoringBlockDirection();
      if (pointInContents.Y() < rect.Y())
      {
        continue;
      }
      if (pointInContents.Y() > rect.MaxY())
      {
        continue;
      }
      closestBox = inline_iterator::BoxIterator {*textBox};
      if (pointInContents.X() < rect.MaxX())
      {
        break;
      }
    }

    if (!closestBox)
    {
      closestBox = lastBox;
    }

    if (!closestBox)
    {
      return CreatePositionWithAffinity(0, Affinity::Downstream);
    }

    return const_cast<RenderObject &>(closestBox->Renderer())
      .PositionForPoint({pointInContents.X(), LayoutUnit(closestBox->VisualRectIgnoringBlockDirection().Y())},
                        source);
  }

  void RenderSVGText::Paint(PaintInfo &paintInfo, const LayoutPoint &) noexcept
  {
    if (paintInfo.Context().PaintingDisabled())
    {
      return;
    }

    if (paintInfo.Phase != PaintPhase::ClippingMask && paintInfo.Phase != PaintPhase::Mask
        && paintInfo.Phase != PaintPhase::Foreground && paintInfo.Phase != PaintPhase::Outline
        && paintInfo.Phase != PaintPhase::SelfOutline)
    {
      return;
    }

    if (!paintInfo.ShouldPaintWithinRoot(*this))
    {
      return;
    }

    PaintInfo blockInfo(paintInfo);
    GraphicsContextStateSaver stateSaver(blockInfo.Context());
    blockInfo.ApplyTransform(LocalToParentTransform());
    RenderBlock::Paint(blockInfo, LayoutPoint());

    // Paint the outlines, if any
    if (paintInfo.Phase == PaintPhase::Foreground)
    {
      blockInfo.Phase = PaintPhase::SelfOutline;
      RenderBlock::Paint(blockInfo, LayoutPoint());
    }
  }

  void RenderSVGText::PaintInlineChildren(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    krys_debug_assert(!paintInfo.Context().PaintingDisabled());

    if (paintInfo.Phase != PaintPhase::Foreground && paintInfo.Phase != PaintPhase::Selection)
    {
      return;
    }

    bool isPrinting = GetDocument().Printing();
    bool hasSelection = !isPrinting && SelectionState() != RenderObject::HighlightState::None;
    bool shouldPaintSelectionHighlight =
      !paintInfo.PaintBehavior.contains(PaintBehavior::SkipSelectionHighlight);

    PaintInfo childPaintInfo(paintInfo);
    childPaintInfo.UpdateSubtreePaintRootForChildren(this);

    auto boxes = inline_iterator::BoxesFor(*this);

    if (hasSelection && shouldPaintSelectionHighlight)
    {
      for (auto &box : boxes)
      {
        if (auto *textBox = DynamicDowncast<inline_iterator::SVGTextBox>(box))
        {
          SVGTextBoxPainter painter(textBox->ContentPath().GetInlineContent(),
                                    textBox->ContentPath().BoxIndex(), paintInfo, paintOffset);
          painter.PaintSelectionBackground();
        }
      }
    }

    List<SVGRenderingContext> contextStack;

    for (auto box = boxes.begin(); box != boxes.end();)
    {
      while (!contextStack.empty() && contextStack.back().Renderer() != box->Renderer().Parent())
      {
        contextStack.pop_back();
      }

      if (auto *textBox = DynamicDowncast<inline_iterator::SVGTextBox>(*box))
      {
        SVGTextBoxPainter painter(textBox->ContentPath().GetInlineContent(),
                                  textBox->ContentPath().BoxIndex(), paintInfo, paintOffset);
        painter.Paint();
      }
      else
      {
        auto *renderer = DynamicDowncast<RenderElement>(box->Renderer());
        contextStack.emplace_back(const_cast<RenderElement &>(*renderer), paintInfo,
                                  SVGRenderingContext::NeedsGraphicsContextSave::SaveGraphicsContext);

        if (!contextStack.back().IsRenderingPrepared() || renderer->HasSelfPaintingLayer())
        {
          box.TraverseLineRightwardOnLineSkippingChildren();
          continue;
        }
      }

      box.TraverseLineRightwardOnLine();
    }

    while (!contextStack.empty())
    {
      contextStack.pop_back();
    }
  }

  RectF RenderSVGText::StrokeBoundingBox() const noexcept
  {
    RectF strokeBoundaries = ObjectBoundingBox();
    if (!Style().HasStroke())
    {
      return strokeBoundaries;
    }

    Ref textElement = ShareRef(TextElement());
    SVGLengthContext lengthContext(textElement.get());
    strokeBoundaries.Inflate(lengthContext.ValueForLength(Style().StrokeWidth(), style::ZoomNeeded {}));
    return strokeBoundaries;
  }

  RectF RenderSVGText::DecoratedBoundingBox() const noexcept
  {
    // The area it may paint, as a mask's buffer must cover it: its fill, stroke and markers with its filter's
    // region, within its clip and mask.
    return RepaintRectInLocalCoordinates(RepaintRectCalculation::Accurate);
  }

  RectF
    RenderSVGText::RepaintRectInLocalCoordinates(RepaintRectCalculation repaintRectCalculation) const noexcept
  {
    auto repaintRect = StrokeBoundingBox();
    SVGRenderSupport::IntersectRepaintRectWithResources(*this, repaintRect, repaintRectCalculation);

    if (auto &textShadow = Style().TextShadow(); !textShadow.IsNone())
    {
      style::AdjustRectForShadow(repaintRect, textShadow, Style().UsedZoomForLength());
    }

    return repaintRect;
  }

  void RenderSVGText::UpdatePositionAndOverflow(const RectF &boundaries) noexcept
  {
    auto boundingRect = EnclosingLayoutRect(boundaries);
    SetLocation(boundingRect.Location());
    SetSize(boundingRect.Size());
    _objectBoundingBox = boundingRect;
    krys_debug_assert(_objectBoundingBox == FrameRect());
  }

  void RenderSVGText::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    auto needsTransformUpdate = [&]()
    {
      if (diff != style::DifferenceResult::Layout)
      {
        return false;
      }

      auto &newStyle = Style();
      if (!oldStyle)
      {
        return newStyle.AffectsTransform();
      }

      return (oldStyle->AffectsTransform() != newStyle.AffectsTransform()
              || oldStyle->Transform() != newStyle.Transform()
              || oldStyle->Translate() != newStyle.Translate() || oldStyle->Scale() != newStyle.Scale()
              || oldStyle->Rotate() != newStyle.Rotate() || oldStyle->OffsetPath() != newStyle.OffsetPath());
    };

    if (needsTransformUpdate())
    {
      SetNeedsTransformUpdate();
    }

    RenderSVGBlock::StyleDidChange(diff, oldStyle);
  }

  bool RenderSVGText::IsObjectBoundingBoxValid() const noexcept
  {
    // Without any lines, the bounding box is invalid.
    return InlineLayout() && InlineLayout()->HasContentfulInlineLine();
  }
}
