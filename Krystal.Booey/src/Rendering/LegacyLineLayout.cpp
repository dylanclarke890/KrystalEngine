#include "Krystal.Booey/Rendering/LegacyLineLayout.hpp"

#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/InlineWalker.hpp"
#include "Krystal.Booey/Rendering/LegacyInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/LegacyRootInlineBox.hpp"
#include "Krystal.Booey/Rendering/Line/BreakingContext.hpp"
#include "Krystal.Booey/Rendering/Line/LineBreaker.hpp"
#include "Krystal.Booey/Rendering/Line/LineInfo.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRootInlineBox.hpp"

namespace krys::boo
{
  LegacyLineLayout::LegacyLineLayout(RenderBlockFlow &flow) noexcept : _flow(flow)
  {
  }

  LegacyLineLayout::~LegacyLineLayout() noexcept
  {
    DeleteLegacyRootBox(true);
  }

  namespace
  {
    void DetermineDirectionality(TextDirection &dir, LegacyInlineIterator iter) noexcept
    {
      while (!iter.AtEnd())
      {
        if (iter.AtParagraphSeparator())
        {
          return;
        }
        if (char32 current = iter.Current())
        {
          UCharDirection charDirection = u_charDirection(static_cast<UChar32>(current));
          if (charDirection == U_LEFT_TO_RIGHT)
          {
            dir = TextDirection::LTR;
            return;
          }
          if (charDirection == U_RIGHT_TO_LEFT || charDirection == U_RIGHT_TO_LEFT_ARABIC)
          {
            dir = TextDirection::RTL;
            return;
          }
        }
        iter.Increment();
      }
    }

    KRYS_NODISCARD UniquePtr<BidiRun> CreateRun(int32 start, int32 end, RenderObject &obj,
                                                InlineBidiResolver &resolver) noexcept
    {
      return CreateUnique<BidiRun>(static_cast<uint32>(start), static_cast<uint32>(end), obj,
                                   resolver.Context(), resolver.Dir());
    }

    void DirtyLineBoxesForRenderer(RenderObject &renderer) noexcept
    {
      if (auto *renderText = DynamicDowncast<RenderSVGInlineText>(renderer))
      {
        renderText->DeleteLegacyLineBoxes();
      }
      else if (auto *renderInline = DynamicDowncast<RenderInline>(renderer))
      {
        renderInline->DeleteLegacyLineBoxes();
      }
    }

    KRYS_NODISCARD bool ParentIsConstructedOrHaveNext(LegacyInlineFlowBox *parentBox) noexcept
    {
      do
      {
        if (parentBox->IsConstructed() || parentBox->NextOnLine())
        {
          return true;
        }
        parentBox = parentBox->Parent();
      } while (parentBox);
      return false;
    }

    void NotifyResolverToResumeInIsolate(InlineBidiResolver &resolver, const RenderInline *root,
                                         RenderObject *startObject) noexcept
    {
      if (root != startObject)
      {
        RenderObject *parent = startObject->Parent();
        NotifyResolverToResumeInIsolate(resolver, root, parent);
        NotifyObserverEnteredObject(&resolver, startObject);
      }
    }

    void SetUpResolverToResumeInIsolate(InlineBidiResolver &resolver, InlineBidiResolver &topResolver,
                                        BidiRun &isolatedRun, const RenderInline *root,
                                        RenderObject *startObject) noexcept
    {
      // Set up _whitespaceCollapsingState
      resolver.GetWhitespaceCollapsingState() = topResolver.GetWhitespaceCollapsingState();
      resolver.GetWhitespaceCollapsingState().SetCurrentTransition(
        topResolver.WhitespaceCollapsingTransitionForIsolatedRun(isolatedRun));

      // Set up _nestedIsolateCount
      NotifyResolverToResumeInIsolate(resolver, root, startObject);
    }

    // FIXME: BidiResolver should have this logic.
    void ConstructBidiRunsForSegment(InlineBidiResolver &topResolver, BidiRunList<BidiRun> &bidiRuns,
                                     const LegacyInlineIterator &endOfRuns,
                                     VisualDirectionOverride override) noexcept
    {
      // FIXME: We should pass a BidiRunList into createBidiRunsForLine instead
      // of the resolver owning the runs.
      krys_debug_assert(&topResolver.Runs() == &bidiRuns);
      krys_debug_assert(topResolver.Position() != endOfRuns);
      topResolver.CreateBidiRunsForLine(endOfRuns, override);

      while (!topResolver.IsolatedRuns().empty())
      {
        // It does not matter which order we resolve the runs as long as we resolve them all.
        auto isolatedRun = topResolver.IsolatedRuns().back();
        topResolver.IsolatedRuns().pop_back();

        RenderObject &startObject = isolatedRun.Object;

        // Only inlines make sense with unicode-bidi: isolate (blocks are already isolated).
        // FIXME: Because enterIsolate is not passed a RenderObject, we have to crawl up the
        // tree to see which parent inline is the isolate. We could change enterIsolate
        // to take a RenderObject and do this logic there, but that would be a layering
        // violation for BidiResolver (which knows nothing about RenderObject).
        RenderInline *isolatedInline =
          Downcast<RenderInline>(HighestContainingIsolateWithinRoot(startObject, &isolatedRun.Root));
        krys_debug_assert(isolatedInline);

        InlineBidiResolver isolatedResolver;
        auto unicodeBidi = isolatedInline->Style().GetUnicodeBidi();
        TextDirection direction = TextDirection::LTR;
        if (unicodeBidi == UnicodeBidi::Plaintext)
        {
          DetermineDirectionality(direction, LegacyInlineIterator(isolatedInline, &isolatedRun.Object, 0));
        }
        else
        {
          krys_debug_assert(unicodeBidi == UnicodeBidi::Isolate
                            || unicodeBidi == UnicodeBidi::IsolateOverride);
          direction = isolatedInline->GetWritingMode().BidiDirection();
        }
        isolatedResolver.SetStatus(BidiStatus(direction, IsOverride(unicodeBidi)));

        SetUpResolverToResumeInIsolate(isolatedResolver, topResolver, isolatedRun.RunToReplace,
                                       isolatedInline, &startObject);

        // The starting position is the beginning of the first run within the isolate that was identified
        // during the earlier call to createBidiRunsForLine. This can be but is not necessarily the
        // first run within the isolate.
        LegacyInlineIterator iter = LegacyInlineIterator(isolatedInline, &startObject, isolatedRun.Position);
        isolatedResolver.SetPositionIgnoringNestedIsolates(iter);

        // We stop at the next end of line; we may re-enter this isolate in the next call to
        // constructBidiRuns().
        isolatedResolver.CreateBidiRunsForLine(endOfRuns, VisualDirectionOverride::NoVisualOverride);
        // Note that we do not delete the runs from the resolver.
        // We're not guaranteed to get any BidiRuns in the previous step. If we don't, we allow the
        // placeholder itself to be turned into an InlineBox. We can't remove it here without potentially
        // losing track of the logically last run.
        if (isolatedResolver.Runs().RunCount())
        {
          bidiRuns.ReplaceRunWithRuns(&isolatedRun.RunToReplace, isolatedResolver.Runs());
        }

        // If we encountered any nested isolate runs, just move them
        // to the top resolver's list for later processing.
        while (!isolatedResolver.IsolatedRuns().empty())
        {
          auto runWithContext = isolatedResolver.IsolatedRuns().back();
          isolatedResolver.IsolatedRuns().pop_back();
          topResolver.SetWhitespaceCollapsingTransitionForIsolatedRun(
            runWithContext.RunToReplace,
            isolatedResolver.WhitespaceCollapsingTransitionForIsolatedRun(runWithContext.RunToReplace));
          topResolver.IsolatedRuns().push_back(runWithContext);
        }
      }
    }

    void RepaintSelfPaintInlineBoxes(const LegacyRootInlineBox &rootInlineBox) noexcept
    {
      if (rootInlineBox.HasSelfPaintInlineBox())
      {
        for (auto *inlineBox = rootInlineBox.FirstChild(); inlineBox; inlineBox = inlineBox->NextOnLine())
        {
          if (auto *renderer = DynamicDowncast<RenderLayerModelObject>(inlineBox->Renderer());
              renderer && renderer->HasSelfPaintingLayer())
          {
            renderer->Repaint();
          }
        }
      }
    }
  }

  bool LegacyLineLayout::ShouldSkipCreatingRunsForObject(RenderObject &object) noexcept
  {
    if (Is<RenderText>(object))
    {
      return false;
    }
    auto &renderElement = Downcast<RenderElement>(object);
    if (renderElement.IsFloating())
    {
      return true;
    }
    if (renderElement.IsOutOfFlowPositioned() && !renderElement.Style().IsOriginalDisplayInlineType()
        && !renderElement.Container()->IsRenderInline())
    {
      return true;
    }
    return false;
  }

  void LegacyLineLayout::AppendRunsForObject(BidiRunList<BidiRun> *runs, int32 start, int32 end,
                                             RenderObject &obj, InlineBidiResolver &resolver) noexcept
  {
    if (start > end || ShouldSkipCreatingRunsForObject(obj))
    {
      return;
    }

    LineWhitespaceCollapsingState &lineWhitespaceCollapsingState = resolver.GetWhitespaceCollapsingState();
    bool haveNextTransition =
      (lineWhitespaceCollapsingState.CurrentTransition() < lineWhitespaceCollapsingState.NumTransitions());
    LegacyInlineIterator nextTransition;
    if (haveNextTransition)
    {
      nextTransition =
        lineWhitespaceCollapsingState.Transitions()[lineWhitespaceCollapsingState.CurrentTransition()];
    }
    if (lineWhitespaceCollapsingState.BetweenTransitions())
    {
      if (!haveNextTransition || (&obj != nextTransition.Renderer()))
      {
        return;
      }
      // This is a new start point. Stop ignoring objects and
      // adjust our start.
      start = static_cast<int32>(nextTransition.Offset());
      lineWhitespaceCollapsingState.IncrementCurrentTransition();
      if (start < end)
      {
        AppendRunsForObject(runs, start, end, obj, resolver);
        return;
      }
    }
    else
    {
      if (!haveNextTransition || (&obj != nextTransition.Renderer()))
      {
        if (runs)
        {
          runs->AppendRun(CreateRun(start, end, obj, resolver));
        }
        return;
      }

      // An end transition has been encountered within our object. We need to append a run with our endpoint.
      if (static_cast<int32>(nextTransition.Offset() + 1) <= end)
      {
        lineWhitespaceCollapsingState.IncrementCurrentTransition();
        // The end of the line is before the object we're inspecting. Skip everything and return
        if (nextTransition.RefersToEndOfPreviousNode())
        {
          return;
        }
        if (static_cast<int32>(nextTransition.Offset() + 1) > start && runs)
        {
          runs->AppendRun(CreateRun(start, static_cast<int32>(nextTransition.Offset() + 1), obj, resolver));
        }
        AppendRunsForObject(runs, static_cast<int32>(nextTransition.Offset() + 1), end, obj, resolver);
      }
      else if (runs)
      {
        runs->AppendRun(CreateRun(start, end, obj, resolver));
      }
    }
  }

  UniquePtr<LegacyRootInlineBox> LegacyLineLayout::CreateRootInlineBox() noexcept
  {
    if (auto *svgText = DynamicDowncast<RenderSVGText>(_flow))
    {
      auto box = CreateUnique<SVGRootInlineBox>(*svgText);
      box->SetHasVirtualLogicalHeight();
      return box;
    }

    return CreateUnique<LegacyRootInlineBox>(_flow);
  }

  LegacyRootInlineBox *LegacyLineLayout::CreateAndAppendRootInlineBox() noexcept
  {
    _legacyRootInlineBox = CreateRootInlineBox();
    // FIXME: Recompute whether the element is ignored (AXObjectCache::deferRecomputeIsIgnored), once there's
    // accessibility.
    return _legacyRootInlineBox.get();
  }

  LegacyInlineBox *LegacyLineLayout::CreateInlineBoxForRenderer(RenderObject *renderer) noexcept
  {
    if (renderer == &_flow)
    {
      return CreateAndAppendRootInlineBox();
    }

    if (auto *textRenderer = DynamicDowncast<RenderSVGInlineText>(*renderer))
    {
      return textRenderer->CreateInlineTextBox();
    }

    if (auto *renderInline = DynamicDowncast<RenderInline>(renderer))
    {
      return renderInline->CreateAndAppendInlineFlowBox();
    }

    krys_debug_assert(false);
    return nullptr;
  }

  LegacyInlineFlowBox *LegacyLineLayout::CreateLineBoxes(RenderObject *obj, const LineInfo &lineInfo,
                                                         LegacyInlineBox *childBox) noexcept
  {
    // See if we have an unconstructed line box for this object that is also
    // the last item on the line.
    uint32 lineDepth = 1;
    LegacyInlineFlowBox *parentBox = nullptr;
    LegacyInlineFlowBox *result = nullptr;
    do
    {
      RenderInline *inlineFlow = obj != &_flow ? &Downcast<RenderInline>(*obj) : nullptr;

      // Get the last box we made for this render object.
      parentBox =
        inlineFlow ? inlineFlow->LastLegacyInlineBox() : Downcast<RenderBlockFlow>(*obj).LegacyRootBox();

      // If this box or its ancestor is constructed then it is from a previous line, and we need
      // to make a new box for our line. If this box or its ancestor is unconstructed but it has
      // something following it on the line, then we know we have to make a new box
      // as well. In this situation our inline has actually been split in two on
      // the same line (this can happen with very fancy language mixtures).
      bool constructedNewBox = false;
      bool canUseExistingParentBox = parentBox && !ParentIsConstructedOrHaveNext(parentBox);
      if (!canUseExistingParentBox)
      {
        // We need to make a new box for this render object. Once
        // made, we need to place it at the end of the current line.
        LegacyInlineBox *newBox = CreateInlineBoxForRenderer(obj);
        parentBox = Downcast<LegacyInlineFlowBox>(newBox);
        parentBox->SetIsFirstLine(lineInfo.IsFirstLine());
        parentBox->SetIsHorizontal(_flow.IsHorizontalWritingMode());
        constructedNewBox = true;
      }

      if (constructedNewBox || canUseExistingParentBox)
      {
        if (!result)
        {
          result = parentBox;
        }

        // If we have hit the block itself, then |box| represents the root
        // inline box for the line, and it doesn't have to be appended to any parent
        // inline.
        if (childBox)
        {
          parentBox->AddToLine(childBox);
        }

        if (!constructedNewBox || obj == &_flow)
        {
          break;
        }

        childBox = parentBox;
      }

      // If we've exceeded our line depth, then jump straight to the root and skip all the remaining
      // intermediate inline flows.
      obj = (++lineDepth >= MaxLineDepth) ? &_flow : obj->Parent();

    } while (true);

    return result;
  }

  LegacyRootInlineBox *LegacyLineLayout::ConstructLine(BidiRunList<BidiRun> &bidiRuns,
                                                       const LineInfo &lineInfo) noexcept
  {
    if (LegacyRootBox())
    {
      // Refuse to create multiple lines for svg content. There should not need to be more than one.
      krys_debug_assert(false);
      return nullptr;
    }

    krys_debug_assert(bidiRuns.FirstRun());
    LegacyInlineFlowBox *parentBox = nullptr;
    for (BidiRun *r = bidiRuns.FirstRun(); r; r = r->Next())
    {
      if (lineInfo.IsEmpty())
      {
        continue;
      }

      LegacyInlineBox *box = CreateInlineBoxForRenderer(&r->Renderer());
      r->SetBox(box);

      // If we have no parent box yet, or if the run is not simply a sibling,
      // then we need to construct inline boxes as necessary to properly enclose the
      // run's inline box. Segments can only be siblings at the root level, as
      // they are positioned separately.
      if (!parentBox || &parentBox->Renderer() != r->Renderer().Parent())
      {
        // Create new inline boxes all the way back to the appropriate insertion point.
        RenderObject *parentToUse = r->Renderer().Parent();
        parentBox = CreateLineBoxes(parentToUse, lineInfo, box);
      }
      else
      {
        // Append the inline box to this line.
        parentBox->AddToLine(box);
      }

      box->SetBidiLevel(r->Level());

      if (auto *textBox = DynamicDowncast<LegacyInlineTextBox>(*box))
      {
        textBox->SetStart(r->Start());
        textBox->SetLen(r->Stop() - r->Start());
      }
    }

    // We should have a root inline box. It should be unconstructed and
    // be the last continuation of our line list.
    krys_debug_assert(LegacyRootBox() && !LegacyRootBox()->IsConstructed());

    // Now mark the line boxes as being constructed.
    LegacyRootBox()->SetConstructed();
    return LegacyRootBox();
  }

  void LegacyLineLayout::RemoveInlineBox(BidiRun &run, const LegacyRootInlineBox &rootLineBox) const noexcept
  {
    auto *inlineBox = run.Box();
    auto *parent = inlineBox->Parent();
    inlineBox->RemoveFromParent();

    auto &renderer = run.Renderer();
    if (auto *textRenderer = DynamicDowncast<RenderSVGInlineText>(renderer))
    {
      textRenderer->RemoveTextBox(Downcast<LegacyInlineTextBox>(*inlineBox));
    }
    delete inlineBox;
    run.SetBox(nullptr);
    // removeFromParent() unnecessarily dirties the ancestor subtree.
    auto *ancestor = parent;
    while (ancestor)
    {
      ancestor->MarkDirty(false);
      if (ancestor == &rootLineBox)
      {
        break;
      }
      ancestor = ancestor->Parent();
    }
  }

  void LegacyLineLayout::RemoveEmptyTextBoxesAndUpdateVisualReordering(LegacyRootInlineBox *lineBox,
                                                                       BidiRun *firstRun) noexcept
  {
    for (auto *run = firstRun; run; run = run->Next())
    {
      if (auto *inlineTextBox = DynamicDowncast<LegacyInlineTextBox>(run->Box()))
      {
        auto &renderer = inlineTextBox->Renderer();
        if (!inlineTextBox->HasTextContent())
        {
          RemoveInlineBox(*run, *lineBox);
          continue;
        }
        if (!inlineTextBox->IsLeftToRightDirection())
        {
          renderer.SetNeedsVisualReordering();
        }
      }
    }
  }

  // This function constructs line boxes for all of the text runs in the resolver and computes their position.
  LegacyRootInlineBox *LegacyLineLayout::CreateLineBoxesFromBidiRuns(uint32 bidiLevel,
                                                                     BidiRunList<BidiRun> &bidiRuns,
                                                                     const LegacyInlineIterator &end,
                                                                     LineInfo &lineInfo) noexcept
  {
    if (!bidiRuns.RunCount())
    {
      return nullptr;
    }

    // FIXME: Why is this only done when we had runs?
    lineInfo.SetLastLine(!end.Renderer());

    LegacyRootInlineBox *lineBox = ConstructLine(bidiRuns, lineInfo);
    if (!lineBox)
    {
      return nullptr;
    }

    lineBox->SetBidiLevel(static_cast<uint8>(bidiLevel));

    RemoveEmptyTextBoxesAndUpdateVisualReordering(lineBox, bidiRuns.FirstRun());

    GlyphOverflowAndFallbackFontsMap textBoxDataMap;
    lineBox->ComputeOverflow(lineBox->LineTop(), lineBox->LineBottom(), textBoxDataMap);

    return lineBox;
  }

  void LegacyLineLayout::LayoutRunsAndFloats(bool hasInlineChild) noexcept
  {
    DeleteLegacyRootBox(true);

    TextDirection direction = Style().GetWritingMode().BidiDirection();
    if (Style().GetUnicodeBidi() == UnicodeBidi::Plaintext)
    {
      DetermineDirectionality(direction,
                              LegacyInlineIterator(&_flow, FirstInlineRendererSkippingEmpty(_flow), 0));
    }

    InlineBidiResolver resolver;
    resolver.SetStatus(BidiStatus(direction, IsOverride(Style().GetUnicodeBidi())));
    LegacyInlineIterator iter =
      LegacyInlineIterator(&_flow, FirstInlineRendererSkippingEmpty(_flow, &resolver), 0);
    resolver.SetPosition(iter, NumberOfIsolateAncestors(iter));

    if (hasInlineChild && !_flow.SelfNeedsLayout())
    {
      _flow.SetNeedsLayout(
        RenderObjectMarkingBehavior::MarkOnlyThis); // Mark as needing a full layout to force us to repaint.
      if (!LayoutContext().NeedsFullRepaint())
      {
        if (auto cachedRect = _flow.CachedLayerClippedOverflowRect())
        {
          // Because we waited until we were already inside layout to discover
          // that the block really needed a full layout, we missed our chance to repaint the layer
          // before layout started. Luckily the layer has cached the repaint rect for its original
          // position and size, and so we can use that to make a repaint happen now.
          _flow.RepaintUsingContainer(_flow.ContainerForRepaint().Renderer, *cachedRect);
        }
      }
    }

    LayoutRunsAndFloatsInRange(resolver);
    if (LegacyRootBox())
    {
      RepaintSelfPaintInlineBoxes(*LegacyRootBox());
    }
  }

  void LegacyLineLayout::LayoutRunsAndFloatsInRange(InlineBidiResolver &resolver) noexcept
  {
    const RenderStyle &styleToUse = Style();
    LineWhitespaceCollapsingState &lineWhitespaceCollapsingState = resolver.GetWhitespaceCollapsingState();
    LegacyInlineIterator end = resolver.Position();
    RenderTextInfo renderTextInfo;

    LineInfo lineInfo {};
    LineBreaker lineBreaker(_flow);

    while (!end.AtEnd())
    {
      lineWhitespaceCollapsingState.Reset();

      lineInfo.SetEmpty(true);
      lineInfo.ResetRunsFromLeadingWhitespace();

      end = lineBreaker.NextLineBreak(resolver, lineInfo, renderTextInfo);
      renderTextInfo.LineBreakIteratorFactory.GetPriorContext().Reset();
      if (resolver.Position().AtEnd())
      {
        // FIXME: We shouldn't be creating any runs in nextLineBreak to begin with!
        // Once BidiRunList is separated from BidiResolver this will not be needed.
        resolver.Runs().Clear();
        resolver
          .MarkCurrentRunEmpty(); // FIXME: This can probably be replaced by an ASSERT (or just removed).
        resolver.SetPosition(LegacyInlineIterator(resolver.Position().Root(), nullptr, 0), 0);
        break;
      }

      krys_debug_assert(end != resolver.Position());

      if (!lineInfo.IsEmpty())
      {
        VisualDirectionOverride override =
          (styleToUse.RtlOrdering() == Order::Visual
             ? (styleToUse.GetWritingMode().IsBidiLTR() ? VisualDirectionOverride::VisualLeftToRightOverride
                                                        : VisualDirectionOverride::VisualRightToLeftOverride)
             : VisualDirectionOverride::NoVisualOverride);

        if (styleToUse.GetUnicodeBidi() == UnicodeBidi::Plaintext && !resolver.Context()->Parent())
        {
          TextDirection direction = styleToUse.GetWritingMode().BidiDirection();
          DetermineDirectionality(direction, resolver.Position());
          resolver.SetStatus(BidiStatus(direction, IsOverride(styleToUse.GetUnicodeBidi())));
        }
        // FIXME: This ownership is reversed. We should own the BidiRunList and pass it to
        // createBidiRunsForLine.
        BidiRunList<BidiRun> &bidiRuns = resolver.Runs();
        ConstructBidiRunsForSegment(resolver, bidiRuns, end, override);
        krys_debug_assert(resolver.Position() == end);

        // Now that the runs have been ordered, we create the line boxes.

        CreateLineBoxesFromBidiRuns(resolver.Status().Context->Level(), bidiRuns, end, lineInfo);

        bidiRuns.Clear();
        resolver
          .MarkCurrentRunEmpty(); // FIXME: This can probably be replaced by an ASSERT (or just removed).
      }

      if (!lineInfo.IsEmpty())
      {
        lineInfo.SetFirstLine(false);
      }

      lineWhitespaceCollapsingState.Reset();
      resolver.SetPosition(end, NumberOfIsolateAncestors(end));
    }
  }

  void LegacyLineLayout::LayoutLineBoxes() noexcept
  {
    _flow.SetLogicalHeight(0_lu);

    DeleteLegacyRootBox();

    if (_flow.FirstChild())
    {
      // In full layout mode, clear the line boxes of children upfront. Otherwise,
      // siblings can run into stale root lineboxes during layout. Then layout
      // the replaced elements later. In partial layout mode, line boxes are not
      // deleted and only dirtied. In that case, we can layout the replaced
      // elements at the same time.
      bool hasInlineChild = false;
      for (InlineWalker walker(_flow); !walker.AtEnd(); walker.Advance())
      {
        RenderObject &o = *walker.Current();

        if (!hasInlineChild && o.IsInline())
        {
          hasInlineChild = true;
        }

        DirtyLineBoxesForRenderer(o);
        o.ClearNeedsLayout();
      }

      LayoutRunsAndFloats(hasInlineChild);
    }

    if (!LegacyRootBox() && _flow.HasLineIfEmpty())
    {
      _flow.SetLogicalHeight(_flow.LogicalHeight() + _flow.LineHeight());
    }
  }

  void LegacyLineLayout::AddOverflowFromInlineChildren() noexcept
  {
    if (auto *rootBox = LegacyRootBox())
    {
      LayoutRect childVisualOverflowRect =
        rootBox->VisualOverflowRect(rootBox->LineTop(), rootBox->LineBottom());
      _flow.AddVisualOverflow(childVisualOverflowRect);
    }
  }

  size_t LegacyLineLayout::LineCount() const noexcept
  {
    return LegacyRootBox() ? 1 : 0;
  }

  const RenderStyle &LegacyLineLayout::Style() const noexcept
  {
    return _flow.Style();
  }

  const LocalFrameViewLayoutContext &LegacyLineLayout::LayoutContext() const noexcept
  {
    return _flow.View().FrameView().LayoutContext();
  }

  void LegacyLineLayout::ShiftLineBy(LayoutUnit shiftX, LayoutUnit shiftY) noexcept
  {
    if (_legacyRootInlineBox)
    {
      _legacyRootInlineBox->AdjustPosition(shiftX.ToFloat(), shiftY.ToFloat());
    }
  }

  void LegacyLineLayout::DeleteLegacyRootBox(bool runCleanup) noexcept
  {
    if (!_legacyRootInlineBox)
    {
      return;
    }

    if (!runCleanup)
    {
      _legacyRootInlineBox = nullptr;
      return;
    }

    auto *rootInlineBoxToDestroy = _legacyRootInlineBox.release();
    rootInlineBoxToDestroy->DeleteLine();
  }
}
