#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/RenderedPosition.hpp"
#include "Krystal.Booey/Editing/TypingCommand.hpp"
#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/SimpleCaretAnimator.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/TreeResolver.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool ShouldAlwaysUseDirectionalSelection(const dom::Document *document) noexcept
    {
      return !document
             || EditingBehavior(document->GetSettings().EditingBehaviorType)
                  .ShouldConsiderSelectionAsDirectional();
    }

    KRYS_NODISCARD EditingBehavior Behavior(const dom::Document *document) noexcept
    {
      return EditingBehavior(document ? document->GetSettings().EditingBehaviorType
                                      : EditingBehaviorType::Windows);
    }

    KRYS_NODISCARD bool SelectionIsOrphanedOrBelongsToWrongDocument(const VisibleSelection &selection,
                                                                    const dom::Document *document) noexcept
    {
      if (selection.IsOrphan())
      {
        return true;
      }
      auto documentOfSelection = selection.GetDocument();
      return document && documentOfSelection && document != documentOfSelection.get();
    }

    KRYS_NODISCARD bool ContainsEndpoints(const dom::Document *document,
                                          const Maybe<dom::SimpleRange> &range) noexcept
    {
      return document && range && document->Contains(range->Start.Container.get())
             && document->Contains(range->End.Container.get());
    }

    KRYS_NODISCARD bool ContainsEndpoints(const dom::Document *document, const dom::Range &liveRange) noexcept
    {
      // Only need to check the start container because live ranges enforce the invariant that start and end
      // have a common ancestor.
      return document && document->Contains(liveRange.StartContainer());
    }

    KRYS_NODISCARD bool IsNonOrphanedCaret(const VisibleSelection &selection) noexcept
    {
      return selection.IsCaret() && !selection.Start().IsOrphan() && !selection.End().IsOrphan();
    }

    KRYS_NODISCARD LayoutBoxExtent ComputeOutsetFromInnerOuterRect(const LayoutRect &innerRect,
                                                                   const LayoutRect &outerRect) noexcept
    {
      LayoutBoxExtent result;
      result.SetLeft(std::max<LayoutUnit>(LayoutUnit(0), innerRect.X() - outerRect.X()));
      result.SetTop(std::max<LayoutUnit>(LayoutUnit(0), innerRect.Y() - outerRect.Y()));
      result.SetRight(std::max<LayoutUnit>(LayoutUnit(0), outerRect.Width() - innerRect.Width()));
      result.SetBottom(std::max<LayoutUnit>(LayoutUnit(0), outerRect.Height() - innerRect.Height()));

      return result;
    }

    void RepaintCaretForLocalRect(dom::Node *node, const LayoutRect &rect,
                                  CaretAnimator *caretAnimator) noexcept
    {
      if (auto *caretPainter = RendererForCaretPainting(node))
      {
        LayoutRect adjustedRect = caretAnimator ? caretAnimator->CaretRepaintRectForLocalRect(rect) : rect;
        if (adjustedRect == rect)
        {
          caretPainter->RepaintRectangle(rect);
        }
        else
        {
          caretPainter->RepaintRectangle(rect, RenderObject::ClipRepaintToLayer::No,
                                         RenderObject::ForceRepaint::Yes,
                                         ComputeOutsetFromInnerOuterRect(rect, adjustedRect));
        }
      }
    }

    KRYS_NODISCARD bool IsBoundary(TextGranularity granularity) noexcept
    {
      return granularity == TextGranularity::LineBoundary || granularity == TextGranularity::ParagraphBoundary
             || granularity == TextGranularity::DocumentBoundary;
    }

    // FIXME: Maybe baseline would be better?
    KRYS_NODISCARD bool AbsoluteCaretY(const VisiblePosition &c, int32 &y) noexcept
    {
      IntRect rect = c.AbsoluteCaretBounds();
      if (rect.IsEmpty())
      {
        return false;
      }
      y = rect.Y() + rect.Height() / 2;
      return true;
    }

    void AdjustEndpointsAtBidiBoundary(VisiblePosition &visibleBase, VisiblePosition &visibleExtent) noexcept
    {
      // FIXME: Consider unifying with the logic in `adjustVisibleExtentPreservingVisualContiguity`, so that
      // we expand the selection to the nearest range that maintains logical and visual contiguity.
      RenderedPosition base(visibleBase);
      RenderedPosition extent(visibleExtent);

      if (base.IsNull() || extent.IsNull() || base.IsEquivalent(extent))
      {
        return;
      }

      if (base.AtLeftBoundaryOfBidiRun())
      {
        if (!extent.AtRightBoundaryOfBidiRun(base.BidiLevelOnRight())
            && base.IsEquivalent(extent.LeftBoundaryOfBidiRun(base.BidiLevelOnRight())))
        {
          visibleBase = VisiblePosition(base.PositionAtLeftBoundaryOfBiDiRun());
          return;
        }
        return;
      }

      if (base.AtRightBoundaryOfBidiRun())
      {
        if (!extent.AtLeftBoundaryOfBidiRun(base.BidiLevelOnLeft())
            && base.IsEquivalent(extent.RightBoundaryOfBidiRun(base.BidiLevelOnLeft())))
        {
          visibleBase = VisiblePosition(base.PositionAtRightBoundaryOfBiDiRun());
          return;
        }
        return;
      }

      if (extent.AtLeftBoundaryOfBidiRun()
          && extent.IsEquivalent(base.LeftBoundaryOfBidiRun(extent.BidiLevelOnRight())))
      {
        visibleExtent = VisiblePosition(extent.PositionAtLeftBoundaryOfBiDiRun());
        return;
      }

      if (extent.AtRightBoundaryOfBidiRun()
          && extent.IsEquivalent(base.RightBoundaryOfBidiRun(extent.BidiLevelOnLeft())))
      {
        visibleExtent = VisiblePosition(extent.PositionAtRightBoundaryOfBiDiRun());
        return;
      }
    }

    KRYS_NODISCARD bool RemovingNodeRemovesPosition(dom::Node &node, const dom::Position &position) noexcept
    {
      if (!position.AnchorNode())
      {
        return false;
      }

      if (position.AnchorNode() == &node)
      {
        return true;
      }

      return Is<dom::Element>(node)
             && dom::TreeQueries::IsShadowIncludingInclusiveAncestor(node, *position.AnchorNode());
    }

    void SetNodeContainsSelectionEndPoint(const dom::Position &position, bool value) noexcept
    {
      // We use AnchorNode instead of GetContainerNode() because NodeWillBeRemoved must update position when
      // anchored node is removed.
      for (dom::Node *currentNode = position.AnchorNode(); currentNode;
           currentNode = currentNode->ParentOrShadowHostNode())
      {
        if (currentNode->ContainsSelectionEndPoint() == value)
        {
          break;
        }
        currentNode->SetContainsSelectionEndPoint(value);
      }
    }

    void UpdatePositionAfterAdoptingTextReplacement(dom::Position &position, dom::CharacterData &node,
                                                    uint32 offset, uint32 oldLength,
                                                    uint32 newLength) noexcept
    {
      if (position.AnchorNode() != &node
          || position.GetAnchorType() != dom::Position::AnchorType::PositionIsOffsetInAnchor)
      {
        return;
      }

      // See: http://www.w3.org/TR/DOM-Level-2-Traversal-Range/ranges.html#Level-2-Range-Mutation
      krys_debug_assert(position.OffsetInContainerNode() >= 0);
      auto positionOffset = static_cast<uint32>(position.OffsetInContainerNode());
      // Replacing text can be viewed as a deletion followed by insertion.
      if (positionOffset >= offset && positionOffset <= offset + oldLength)
      {
        position.MoveToOffset(offset);
      }

      // Adjust the offset if the position is after the end of the deleted contents
      // (positionOffset > offset + oldLength) to avoid having a stale offset.
      if (positionOffset > offset + oldLength)
      {
        position.MoveToOffset(positionOffset - oldLength + newLength);
      }

      krys_debug_assert(static_cast<uint32>(position.OffsetInContainerNode()) <= node.Length());
    }
  }

  FrameSelection::FrameSelection(dom::Document *document) noexcept
      : _document(document), _caretAnimator(CreateUnique<SimpleCaretAnimator>(*this)),
        _focused(document && document->IsFocusedAndActive())
  {
    if (ShouldAlwaysUseDirectionalSelection(_document))
    {
      _selection.SetDirectionality(Directionality::Strong);
    }

    bool activeAndFocused = IsFocusedAndActive();

    constexpr auto shouldUpdateAppearance = ShouldUpdateAppearance::No;

    // Caret blinking (blinks | does not blink)
    if (activeAndFocused)
    {
      SetSelectionFromNone();
      RemoveCaretVisibilitySuppressionReason(CaretVisibilitySuppressionReason::IsNotFocusedOrActive,
                                             shouldUpdateAppearance);
    }
    else
    {
      AddCaretVisibilitySuppressionReason(CaretVisibilitySuppressionReason::IsNotFocusedOrActive,
                                          shouldUpdateAppearance);
    }
  }

  FrameSelection::~FrameSelection() noexcept = default;

  dom::Element *FrameSelection::RootEditableElementOrDocumentElement() const noexcept
  {
    auto *selectionRoot = _selection.RootEditableElement();
    return selectionRoot ? selectionRoot : _document->DocumentElement().get();
  }

  void FrameSelection::MoveTo(const VisiblePosition &position, UserTriggered userTriggered,
                              CursorAlignOnScroll align) noexcept
  {
    SetSelection(VisibleSelection(position.DeepEquivalent(), position.DeepEquivalent(),
                                  position.GetAffinity(), _selection.GetDirectionality()),
                 DefaultSetSelectionOptions(userTriggered), align);
  }

  void FrameSelection::MoveTo(const VisiblePosition &base, const VisiblePosition &extent,
                              UserTriggered userTriggered) noexcept
  {
    SetSelection(VisibleSelection(base.DeepEquivalent(), extent.DeepEquivalent(), base.GetAffinity(),
                                  Directionality::Strong),
                 DefaultSetSelectionOptions(userTriggered));
  }

  void FrameSelection::MoveTo(const dom::Position &position, Affinity affinity,
                              UserTriggered userTriggered) noexcept
  {
    SetSelection(VisibleSelection(position, affinity, _selection.GetDirectionality()),
                 DefaultSetSelectionOptions(userTriggered));
  }

  void FrameSelection::MoveTo(const dom::Position &base, const dom::Position &extent, Affinity affinity,
                              UserTriggered userTriggered) noexcept
  {
    SetSelection(VisibleSelection(base, extent, affinity, Directionality::Strong),
                 DefaultSetSelectionOptions(userTriggered));
  }

  void FrameSelection::MoveWithoutValidationTo(const dom::Position &base, const dom::Position &extent,
                                               bool selectionHasDirection,
                                               EnumSet<SetSelectionOption> options) noexcept
  {
    VisibleSelection newSelection;
    newSelection.SetWithoutValidation(base, extent);
    newSelection.SetDirectionality(selectionHasDirection ? Directionality::Strong : Directionality::None);
    SetSelection(newSelection, options, CursorAlignOnScroll::IfNeeded, TextGranularity::CharacterGranularity);
  }

  void
    FrameSelection::SetSelectionByMouseIfDifferent(const VisibleSelection &passedNewSelection,
                                                   TextGranularity granularity,
                                                   EndPointsAdjustmentMode endpointsAdjustmentMode) noexcept
  {
    VisibleSelection newSelection = passedNewSelection;
    auto directionality = ShouldAlwaysUseDirectionalSelection(_document) ? Directionality::Strong
                                                                         : newSelection.GetDirectionality();

    VisiblePosition base = _originalBase.IsNotNull() ? _originalBase : newSelection.VisibleBase();
    VisiblePosition newBase = base;
    VisiblePosition extent = newSelection.VisibleExtent();
    VisiblePosition newExtent = extent;
    if (endpointsAdjustmentMode == EndPointsAdjustmentMode::AdjustAtBidiBoundary)
    {
      AdjustEndpointsAtBidiBoundary(newBase, newExtent);
    }

    if (newBase != base || newExtent != extent)
    {
      _originalBase = base;
      newSelection.SetBase(newBase);
      newSelection.SetExtent(newExtent);
    }
    else if (_originalBase.IsNotNull())
    {
      if (_selection.Base() == newSelection.Base())
      {
        newSelection.SetBase(_originalBase);
      }
      _originalBase = {};
    }

    newSelection.SetDirectionality(directionality);
    if (_selection == newSelection || !ShouldChangeSelection(newSelection))
    {
      return;
    }

    auto options = DefaultSetSelectionOptions();
    options.Add(SetSelectionOption::FireSelectEvent);
    SetSelection(newSelection, options, CursorAlignOnScroll::IfNeeded, granularity);
  }

  bool FrameSelection::SetSelectionWithoutUpdatingAppearance(
    const VisibleSelection &newSelectionPossiblyWithoutDirection, EnumSet<SetSelectionOption> options,
    CursorAlignOnScroll, TextGranularity granularity) noexcept
  {
    auto *document = _document;
    VisibleSelection newSelection = newSelectionPossiblyWithoutDirection;
    if (ShouldAlwaysUseDirectionalSelection(document))
    {
      newSelection.SetDirectionality(Directionality::Strong);
    }

    VisibleSelection oldSelection = _selection;
    bool willMutateSelection = oldSelection != newSelection;

    if (newSelection.IsOrphan())
    {
      krys_debug_assert(false);
      Clear();
      return false;
    }

    if (!document || (!document->Frame() && !newSelection.GetDocument()))
    {
      SetNodeFlags(_selection, false);
      _selection = newSelection;
      SetNodeFlags(_selection, true);
      UpdateOrDisassociateLiveRange(options.Contains(SetSelectionOption::MaintainLiveRange));
      return false;
    }

    bool selectionEndpointsBelongToMultipleDocuments =
      newSelection.Base().GetDocument() && !newSelection.GetDocument();
    bool selectionIsInAnotherDocument =
      newSelection.GetDocument() && newSelection.GetDocument().get() != document;
    bool selectionIsInDetachedDocument = newSelection.GetDocument() && !newSelection.GetDocument()->Frame();
    if (selectionEndpointsBelongToMultipleDocuments || selectionIsInAnotherDocument
        || selectionIsInDetachedDocument)
    {
      Clear();
      return false;
    }
    krys_debug_assert(document->Frame());

    if (options.Contains(SetSelectionOption::CloseTyping))
    {
      TypingCommand::CloseTyping(*document);
    }
    if (options.Contains(SetSelectionOption::ClearTypingStyle))
    {
      ClearTypingStyle();
    }

    _granularity = granularity;
    SetNodeFlags(_selection, false);
    _selection = newSelection;
    SetNodeFlags(_selection, true);
    UpdateOrDisassociateLiveRange(options.Contains(SetSelectionOption::MaintainLiveRange));

    // Selection offsets should increase when LF is inserted before the caret in InsertLineBreakCommand. See
    // <https://webkit.org/b/56061>.
    // https://www.w3.org/TR/selection-api/#selectionchange-event
    RefPtr<html::HTMLTextFormControlElement> textControl =
      ShareRefPtr(html::EnclosingTextFormControl(newSelection.Start()));
    bool shouldScheduleSelectionChangeEvent = willMutateSelection;
    if (textControl)
    {
      shouldScheduleSelectionChangeEvent =
        textControl->SelectionChanged(options.Contains(SetSelectionOption::FireSelectEvent));
    }

    if (!willMutateSelection)
    {
      return false;
    }

    SetCaretRectNeedsUpdate();

    if (!newSelection.IsNone() && !options.Contains(SetSelectionOption::DoNotSetFocus))
    {
      auto *oldFocusedElement = document->FocusedElement();
      SetFocusedElementIfNeeded(options);
      if (!document->Frame())
      {
        return false;
      }
      // FIXME: Should not be needed.
      if (document->FocusedElement() != oldFocusedElement)
      {
        document->UpdateStyleIfNeeded();
      }
    }

    // Always clear the x position used for vertical arrow navigation.
    // It will be restored by the vertical arrow navigation code if necessary.
    _xPosForVerticalArrowNavigation = null;
    if (!options.Contains(SetSelectionOption::DoNotNotifyEditorClients))
    {
      document->GetEditor().RespondToChangedSelection(oldSelection, options);
    }

    if (shouldScheduleSelectionChangeEvent && textControl)
    {
      textControl->ScheduleSelectionChangeEvent();
    }
    else if (shouldScheduleSelectionChangeEvent && !_hasScheduledSelectionChangeEventOnDocument)
    {
      _hasScheduledSelectionChangeEventOnDocument = true;
      html::EventLoop::Current().QueueTask(
        html::TaskSource::UserInteraction, *document,
        [weakDocument = WeakPtr<dom::Document>::WithRef(*document)]
        {
          if (RefPtr<dom::Document> document = ShareRefPtr(weakDocument.get()))
          {
            document->Selection()._hasScheduledSelectionChangeEventOnDocument = false;
            (void)dom::EventDispatcher::FireEvent(dom::DOMStringAtom(EventNames::SelectionChange), *document);
          }
        });
    }

    return true;
  }

  void FrameSelection::SetSelection(const VisibleSelection &selection, EnumSet<SetSelectionOption> options,
                                    CursorAlignOnScroll align, TextGranularity granularity) noexcept
  {
    RefPtr<dom::Document> document = ShareRefPtr(_document);
    if (!SetSelectionWithoutUpdatingAppearance(selection, options, align, granularity))
    {
      return;
    }

    // FIXME: Revealing up to the main frame and delegating the main frame's scroll, once there are frames.
    if (options.Contains(SetSelectionOption::RevealSelectionUpToMainFrame))
    {
      _selectionRevealMode = SelectionRevealMode::RevealUpToMainFrame;
    }
    else if (options.Contains(SetSelectionOption::RevealSelection))
    {
      _selectionRevealMode = SelectionRevealMode::Reveal;
    }
    else if (options.Contains(SetSelectionOption::DelegateMainFrameScroll))
    {
      _selectionRevealMode = SelectionRevealMode::DelegateMainFrameScroll;
    }
    else
    {
      _selectionRevealMode = SelectionRevealMode::DoNotReveal;
    }
    _alwaysAlignCursorOnScrollWhenRevealingSelection = align == CursorAlignOnScroll::Always;

    _pendingSelectionUpdate = true;

    document->ScheduleContentRelevancyUpdate(dom::ContentRelevancy::Selected);

    // The rendering update brings the render tree's selection up to date; schedule one.
    if (auto *view = document->View())
    {
      view->ScheduleSelectionUpdate();
    }

    if (document->HasPendingStyleRecalc())
    {
      return;
    }

    auto *frameView = document->View();
    if (frameView && frameView->LayoutContext().IsLayoutPending())
    {
      return;
    }

    if (!options.Contains(SetSelectionOption::IsUserTriggered))
    {
      return;
    }

    UpdateAndRevealSelection(
      options.Contains(SetSelectionOption::SmoothScroll) ? ScrollBehavior::Smooth : ScrollBehavior::Instant,
      options.Contains(SetSelectionOption::RevealSelectionBounds) ? RevealExtentOption::DoNotRevealExtent
                                                                  : RevealExtentOption::RevealExtent,
      options.Contains(SetSelectionOption::ForceCenterScroll) ? ForceCenterScroll::Yes
                                                              : ForceCenterScroll::No);
  }

  void FrameSelection::UpdateSelectionAppearanceNow() noexcept
  {
    auto *document = _document;
    if (!document || !document->GetRenderView())
    {
      return;
    }

    document->UpdateStyleIfNeeded();
    if (_pendingSelectionUpdate)
    {
      UpdateAppearance();
    }
  }

  void FrameSelection::SetNeedsSelectionUpdate() noexcept
  {
    _pendingSelectionUpdate = true;
    if (auto *view = _document->GetRenderView())
    {
      view->Selection().Clear();
    }
  }

  // FIXME: Accessibility notifications, with accessibility.
  void FrameSelection::UpdateAndRevealSelection(ScrollBehavior scrollBehavior,
                                                RevealExtentOption revealExtent,
                                                ForceCenterScroll forceCenterScroll) noexcept
  {
    if (!_pendingSelectionUpdate)
    {
      return;
    }

    _pendingSelectionUpdate = false;

    UpdateAppearance();

    if (_selectionRevealMode != SelectionRevealMode::DoNotReveal)
    {
      ScrollAlignment alignment;

      if (Behavior(_document).ShouldCenterAlignWhenSelectionIsRevealed())
      {
        alignment = _alwaysAlignCursorOnScrollWhenRevealingSelection ? ScrollAlignment::AlignCenterAlways
                                                                     : ScrollAlignment::AlignCenterIfNeeded;
      }
      else
      {
        alignment = _alwaysAlignCursorOnScrollWhenRevealingSelection ? ScrollAlignment::AlignTopAlways
                                                                     : ScrollAlignment::AlignToEdgeIfNeeded;
      }

      if (forceCenterScroll == ForceCenterScroll::Yes)
      {
        alignment = ScrollAlignment::AlignCenterAlways;
      }

      RevealSelection({_selectionRevealMode, alignment, revealExtent, scrollBehavior});
    }
  }

  void FrameSelection::RevealSelection(const RevealSelectionOptions &revealSelectionOptions) noexcept
  {
    if (revealSelectionOptions.RevealMode == SelectionRevealMode::DoNotReveal)
    {
      return;
    }

    if (IsNone())
    {
      return;
    }

    UpdateSelectionAppearanceNow();

    LayoutRect rect;
    bool insideFixed = false;
    if (IsCaret())
    {
      rect = LayoutRect(AbsoluteCaretBounds(&insideFixed));
    }
    else
    {
      rect = revealSelectionOptions.RevealExtent == RevealExtentOption::RevealExtent
               ? LayoutRect(VisiblePosition(_selection.Extent()).AbsoluteCaretBounds())
               : LayoutRect(EnclosingIntRect(SelectionBounds(ClipToVisibleContent::No)));
    }

    dom::Position start = _selection.Start();
    krys_debug_assert(start.DeprecatedNode());
    if (!start.DeprecatedNode() || !start.DeprecatedNode()->Renderer())
    {
      return;
    }

    _selectionRevealMode = SelectionRevealMode::DoNotReveal;

    // FIXME: This code only handles scrolling the startContainer's layer, but
    // the selection rect could intersect more than just that.
    if (auto *view = _document->Frame() ? _document->Frame()->View() : nullptr)
    {
      view->SetLastUserScrollType(LocalFrameView::UserScrollType::Implicit);
    }
    ScrollRectToVisibleOptions options;
    options.RevealMode = revealSelectionOptions.RevealMode;
    options.AlignX = revealSelectionOptions.Alignment;
    options.AlignY = revealSelectionOptions.Alignment;
    options.Behavior = revealSelectionOptions.Behavior;
    (void)LocalFrameView::ScrollRectToVisible(rect, *start.DeprecatedNode()->Renderer(), insideFixed,
                                              options);
    UpdateAppearance();
  }

  void FrameSelection::SetNodeFlags(VisibleSelection &selection, bool value) noexcept
  {
    if (!_document)
    {
      return;
    }
    SetNodeContainsSelectionEndPoint(selection.Anchor(), value);
    SetNodeContainsSelectionEndPoint(selection.Focus(), value);
    SetNodeContainsSelectionEndPoint(selection.Base(), value);
    SetNodeContainsSelectionEndPoint(selection.Extent(), value);
    SetNodeContainsSelectionEndPoint(selection.Start(), value);
    SetNodeContainsSelectionEndPoint(selection.End(), value);
  }

  void FrameSelection::NodeWillBeRemoved(dom::Node &node) noexcept
  {
    // There can't be a selection inside a fragment, so if a fragment's node is being removed,
    // the selection in the document that created the fragment needs no adjustment.
    if (!node.IsConnected())
    {
      return;
    }

    if (!node.ContainsSelectionEndPoint())
    {
      krys_debug_assert(!RemovingNodeRemovesPosition(node, _selection.Anchor()));
      krys_debug_assert(!RemovingNodeRemovesPosition(node, _selection.Focus()));
      krys_debug_assert(!RemovingNodeRemovesPosition(node, _selection.Base()));
      krys_debug_assert(!RemovingNodeRemovesPosition(node, _selection.Extent()));
      krys_debug_assert(!RemovingNodeRemovesPosition(node, _selection.Start()));
      krys_debug_assert(!RemovingNodeRemovesPosition(node, _selection.End()));
      return;
    }

    RespondToNodeModification(node, RemovingNodeRemovesPosition(node, _selection.Anchor()),
                              RemovingNodeRemovesPosition(node, _selection.Focus()),
                              RemovingNodeRemovesPosition(node, _selection.Base()),
                              RemovingNodeRemovesPosition(node, _selection.Extent()),
                              RemovingNodeRemovesPosition(node, _selection.Start()),
                              RemovingNodeRemovesPosition(node, _selection.End()));

    if (_previousCaretNode && dom::TreeQueries::IsInclusiveAncestor(node, *_previousCaretNode)) KRYS_UNLIKELY
    {
      _previousCaretNode = ShareRefPtr(_selection.Start().AnchorNode());
      SetCaretRectNeedsUpdate();
    }
  }

  void FrameSelection::RespondToNodeModification(dom::Node &node, bool anchorRemoved, bool focusRemoved,
                                                 bool baseRemoved, bool extentRemoved, bool startRemoved,
                                                 bool endRemoved) noexcept
  {
    bool clearRenderTreeSelection = false;
    bool clearDOMTreeSelection = false;

    if (anchorRemoved || focusRemoved)
    {
      dom::Position anchor = _selection.Anchor();
      dom::Position focus = _selection.Focus();
      if (anchorRemoved)
      {
        UpdatePositionForNodeRemoval(anchor, node);
      }
      if (focusRemoved)
      {
        UpdatePositionForNodeRemoval(focus, node);
      }

      if (anchor.IsNotNull() && focus.IsNotNull())
      {
        SetNodeFlags(_selection, false);
        _selection.SetWithoutValidation(anchor, focus);
        SetNodeFlags(_selection, true);
      }
      else
      {
        clearDOMTreeSelection = true;
      }

      clearRenderTreeSelection = true;
    }
    if (startRemoved || endRemoved)
    {
      dom::Position start = _selection.Start();
      dom::Position end = _selection.End();
      if (startRemoved)
      {
        UpdatePositionForNodeRemoval(start, node);
      }
      if (endRemoved)
      {
        UpdatePositionForNodeRemoval(end, node);
      }

      if (start.IsNotNull() && end.IsNotNull())
      {
        SetNodeFlags(_selection, false);
        if (_selection.IsBaseFirst())
        {
          _selection.SetWithoutValidation(start, end);
        }
        else
        {
          _selection.SetWithoutValidation(end, start);
        }
        SetNodeFlags(_selection, true);
      }
      else
      {
        clearDOMTreeSelection = true;
      }

      clearRenderTreeSelection = true;
    }
    else if (baseRemoved || extentRemoved)
    {
      // The base and/or extent are about to be removed, but the start and end aren't.
      // Change the base and extent to the start and end, but don't re-validate the
      // selection, since doing so could move the start and end into the node
      // that is about to be removed.
      SetNodeFlags(_selection, false);
      if (_selection.IsBaseFirst())
      {
        _selection.SetWithoutValidation(_selection.Start(), _selection.End());
      }
      else
      {
        _selection.SetWithoutValidation(_selection.End(), _selection.Start());
      }
      SetNodeFlags(_selection, true);
    }
    else if (IsRange())
    {
      if (auto range = _selection.FirstRange(); range && dom::Intersects(*range, node))
      {
        // If we did nothing here, when this node's renderer was destroyed, the rect that it
        // occupied would be invalidated, but, selection gaps that change as a result of
        // the removal wouldn't be invalidated.
        // FIXME: Don't do so much unnecessary invalidation.
        clearRenderTreeSelection = true;
      }
    }

    if (clearRenderTreeSelection)
    {
      if (auto *renderView = node.NodeDocument().GetRenderView())
      {
        renderView->Selection().Clear();

        // Trigger a selection update so the selection will be set again.
        _pendingSelectionUpdate = true;
        renderView->FrameView().ScheduleSelectionUpdate();
      }
    }

    if (clearDOMTreeSelection)
    {
      SetSelection(VisibleSelection(),
                   {SetSelectionOption::DoNotSetFocus, SetSelectionOption::MaintainLiveRange});
    }
  }

  void FrameSelection::TextWasReplaced(dom::CharacterData &node, uint32 offset, uint32 oldLength,
                                       uint32 newLength) noexcept
  {
    if (IsNone() || !node.IsConnected())
    {
      return;
    }

    dom::Position anchor = _selection.Anchor();
    dom::Position focus = _selection.Focus();
    dom::Position base = _selection.Base();
    dom::Position extent = _selection.Extent();
    dom::Position start = _selection.Start();
    dom::Position end = _selection.End();
    UpdatePositionAfterAdoptingTextReplacement(anchor, node, offset, oldLength, newLength);
    UpdatePositionAfterAdoptingTextReplacement(focus, node, offset, oldLength, newLength);
    UpdatePositionAfterAdoptingTextReplacement(base, node, offset, oldLength, newLength);
    UpdatePositionAfterAdoptingTextReplacement(extent, node, offset, oldLength, newLength);
    UpdatePositionAfterAdoptingTextReplacement(start, node, offset, oldLength, newLength);
    UpdatePositionAfterAdoptingTextReplacement(end, node, offset, oldLength, newLength);

    if ((anchor != _selection.Anchor() || focus != _selection.Focus()) || base != _selection.Base()
        || extent != _selection.Extent() || start != _selection.Start() || end != _selection.End())
    {
      VisibleSelection newSelection;
      newSelection.SetWithoutValidation(anchor, focus);

      SetSelection(newSelection, {SetSelectionOption::DoNotSetFocus, SetSelectionOption::MaintainLiveRange});
    }
  }

  void FrameSelection::Clear() noexcept
  {
    _granularity = TextGranularity::CharacterGranularity;
    SetSelection(VisibleSelection());
  }

  void FrameSelection::WillBeRemovedFromFrame() noexcept
  {
    _granularity = TextGranularity::CharacterGranularity;

    GetCaretAnimator().Stop();

    if (auto *view = _document->GetRenderView())
    {
      view->Selection().Clear();
    }

    auto options = DefaultSetSelectionOptions();
    options.Add(SetSelectionOption::DoNotNotifyEditorClients);
    SetSelectionWithoutUpdatingAppearance(VisibleSelection(), options, CursorAlignOnScroll::IfNeeded,
                                          TextGranularity::CharacterGranularity);
    _previousCaretNode = nullptr;
    _originalBase = {};
  }

  TextDirection FrameSelection::DirectionOfEnclosingBlock() noexcept
  {
    return boo::DirectionOfEnclosingBlock(_selection.Extent());
  }

  TextDirection FrameSelection::DirectionOfSelection() noexcept
  {
    // Get bot VisiblePositions first because VisibleStart() and VisibleEnd()
    // can cause layout, which has the potential to invalidate lineboxes.
    auto startPosition = _selection.VisibleStart();
    auto endPosition = _selection.VisibleEnd();
    auto startBox = startPosition.GetInlineBoxAndOffset().Box;
    auto endBox = endPosition.GetInlineBoxAndOffset().Box;
    if (startBox && endBox && startBox->Direction() == endBox->Direction())
    {
      return startBox->Direction();
    }
    return DirectionOfEnclosingBlock();
  }

  void FrameSelection::WillBeModified(Alteration alter, SelectionDirection direction) noexcept
  {
    if (alter != Alteration::Extend)
    {
      return;
    }

    dom::Position start = _selection.Start();
    dom::Position end = _selection.End();

    bool baseIsStart = true;

    if (_selection.GetDirectionality() == Directionality::Strong)
    {
      // Make base and extent match start and end so we extend the user-visible selection.
      // This only matters for cases where base and extent point to different positions than
      // start and end (e.g. after a double-click to select a word).
      baseIsStart = _selection.IsBaseFirst();
    }
    else
    {
      switch (direction)
      {
        case SelectionDirection::Right:    baseIsStart = DirectionOfSelection() == TextDirection::LTR; break;
        case SelectionDirection::Forward:  baseIsStart = true; break;
        case SelectionDirection::Left:     baseIsStart = DirectionOfSelection() != TextDirection::LTR; break;
        case SelectionDirection::Backward: baseIsStart = false; break;
      }
    }
    SetNodeFlags(_selection, false);
    if (baseIsStart)
    {
      _selection.SetBase(start);
      _selection.SetExtent(end);
    }
    else
    {
      _selection.SetBase(end);
      _selection.SetExtent(start);
    }
    SetNodeFlags(_selection, true);
    if (SelectionIsOrphanedOrBelongsToWrongDocument(_selection, _document))
    {
      Clear();
    }
  }

  VisiblePosition FrameSelection::PositionForPlatform(bool isGetStart) const noexcept
  {
    // FIXME: VisibleSelection should be fixed to ensure as an invariant that
    // base/extent always point to the same nodes as start/end, but which points
    // to which depends on the value of IsBaseFirst. Then this can be changed
    // to just return _selection.Extent().
    if (Behavior(_document).ShouldAlwaysExtendSelectionFromExtentEndpoint())
    {
      return _selection.IsBaseFirst() ? _selection.VisibleEnd() : _selection.VisibleStart();
    }

    return isGetStart ? _selection.VisibleStart() : _selection.VisibleEnd();
  }

  VisiblePosition FrameSelection::StartForPlatform() const noexcept
  {
    return PositionForPlatform(true);
  }

  VisiblePosition FrameSelection::EndForPlatform() const noexcept
  {
    return PositionForPlatform(false);
  }

  VisiblePosition
    FrameSelection::NextWordPositionForPlatform(const VisiblePosition &originalPosition) noexcept
  {
    VisiblePosition positionAfterCurrentWord = NextWordPosition(originalPosition);

    if (Behavior(_document).ShouldSkipSpaceWhenMovingRight())
    {
      // In order to skip spaces when moving right, we advance one
      // word further and then move one word back. Given the
      // semantics of PreviousWordPosition() this will put us at the
      // beginning of the word following.
      VisiblePosition positionAfterSpacingAndFollowingWord = NextWordPosition(positionAfterCurrentWord);
      if (positionAfterSpacingAndFollowingWord != positionAfterCurrentWord)
      {
        positionAfterCurrentWord = PreviousWordPosition(positionAfterSpacingAndFollowingWord);
      }

      bool movingBackwardsMovedPositionToStartOfCurrentWord =
        positionAfterCurrentWord == PreviousWordPosition(NextWordPosition(originalPosition));
      if (movingBackwardsMovedPositionToStartOfCurrentWord)
      {
        positionAfterCurrentWord = positionAfterSpacingAndFollowingWord;
      }
    }
    return positionAfterCurrentWord;
  }

  void FrameSelection::AdjustSelectionExtentIfNeeded(VisiblePosition &extent, bool isForward,
                                                     UserTriggered) noexcept
  {
    if (RefPtr<dom::Node> rootUserSelectAll =
          dom::Position::RootUserSelectAllForNode(extent.DeepEquivalent().AnchorNode()))
    {
      extent = isForward
                 ? VisiblePosition(dom::PositionAfterNode(rootUserSelectAll.get())
                                     .Downstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary))
                 : VisiblePosition(dom::PositionBeforeNode(rootUserSelectAll.get())
                                     .Upstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary));
    }
  }

  VisiblePosition FrameSelection::ModifyExtendingRight(TextGranularity granularity,
                                                       UserTriggered userTriggered) noexcept
  {
    VisiblePosition pos(_selection.Extent(), _selection.GetAffinity());

    // The difference between ModifyExtendingRight and ModifyExtendingForward is:
    // ModifyExtendingForward always extends forward logically.
    // ModifyExtendingRight behaves the same as ModifyExtendingForward except for extending character or word,
    // it extends forward logically if the enclosing block is TextDirection::LTR,
    // but it extends backward logically if the enclosing block is TextDirection::RTL.
    switch (granularity)
    {
      case TextGranularity::CharacterGranularity:
        if (DirectionOfEnclosingBlock() == TextDirection::LTR)
        {
          pos = pos.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
        }
        else
        {
          pos = pos.Previous(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
        }
        break;
      case TextGranularity::WordGranularity:
        if (DirectionOfEnclosingBlock() == TextDirection::LTR)
        {
          pos = NextWordPositionForPlatform(pos);
        }
        else
        {
          pos = PreviousWordPosition(pos);
        }
        break;
      case TextGranularity::LineBoundary:
        if (DirectionOfEnclosingBlock() == TextDirection::LTR)
        {
          pos = ModifyExtendingForward(granularity, userTriggered);
        }
        else
        {
          pos = ModifyExtendingBackward(granularity, userTriggered);
        }
        break;
      case TextGranularity::SentenceGranularity:
      case TextGranularity::LineGranularity:
      case TextGranularity::ParagraphGranularity:
      case TextGranularity::SentenceBoundary:
      case TextGranularity::ParagraphBoundary:
      case TextGranularity::DocumentBoundary:
        // FIXME: implement all of the above?
        pos = ModifyExtendingForward(granularity, userTriggered);
        break;
      case TextGranularity::DocumentGranularity: krys_debug_assert(false); break;
    }
    AdjustSelectionExtentIfNeeded(pos, DirectionOfEnclosingBlock() == TextDirection::LTR, userTriggered);
    return pos;
  }

  VisiblePosition FrameSelection::ModifyExtendingForward(TextGranularity granularity,
                                                         UserTriggered userTriggered) noexcept
  {
    VisiblePosition pos(_selection.Extent(), _selection.GetAffinity());
    switch (granularity)
    {
      case TextGranularity::CharacterGranularity:
        pos = pos.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
        break;
      case TextGranularity::WordGranularity:     pos = NextWordPositionForPlatform(pos); break;
      case TextGranularity::SentenceGranularity: pos = NextSentencePosition(pos); break;
      case TextGranularity::LineGranularity:
        pos = NextLinePosition(pos, LineDirectionPointForBlockDirectionNavigation(PositionType::Extent));
        break;
      case TextGranularity::ParagraphGranularity:
        pos = NextParagraphPosition(pos, LineDirectionPointForBlockDirectionNavigation(PositionType::Extent));
        break;
      case TextGranularity::DocumentGranularity: krys_debug_assert(false); break;
      case TextGranularity::SentenceBoundary:    pos = EndOfSentence(EndForPlatform()); break;
      case TextGranularity::LineBoundary:        pos = LogicalEndOfLine(EndForPlatform()); break;
      case TextGranularity::ParagraphBoundary:   pos = EndOfParagraph(EndForPlatform()); break;
      case TextGranularity::DocumentBoundary:
        pos = EndForPlatform();
        if (IsEditablePosition(pos.DeepEquivalent()))
        {
          pos = EndOfEditableContent(pos);
        }
        else
        {
          pos = EndOfDocument(pos);
        }
        break;
    }
    AdjustSelectionExtentIfNeeded(pos, DirectionOfEnclosingBlock() == TextDirection::LTR, userTriggered);
    return pos;
  }

  VisiblePosition FrameSelection::ModifyMovingRight(TextGranularity granularity,
                                                    bool *reachedBoundary) noexcept
  {
    if (reachedBoundary)
    {
      *reachedBoundary = false;
    }
    VisiblePosition pos;
    switch (granularity)
    {
      case TextGranularity::CharacterGranularity:
        if (IsRange())
        {
          if (DirectionOfSelection() == TextDirection::LTR)
          {
            pos = VisiblePosition(_selection.End(), _selection.GetAffinity());
          }
          else
          {
            pos = VisiblePosition(_selection.Start(), _selection.GetAffinity());
          }
        }
        else
        {
          pos = VisiblePosition(_selection.Extent(), _selection.GetAffinity()).Right(true, reachedBoundary);
        }
        break;
      case TextGranularity::WordGranularity:
      {
        bool skipsSpaceWhenMovingRight = Behavior(_document).ShouldSkipSpaceWhenMovingRight();
        VisiblePosition currentPosition(_selection.Extent(), _selection.GetAffinity());
        pos = RightWordPosition(currentPosition, skipsSpaceWhenMovingRight);
        if (reachedBoundary)
        {
          *reachedBoundary = pos == currentPosition;
        }
        break;
      }
      case TextGranularity::SentenceGranularity:
      case TextGranularity::LineGranularity:
      case TextGranularity::ParagraphGranularity:
      case TextGranularity::SentenceBoundary:
      case TextGranularity::ParagraphBoundary:
      case TextGranularity::DocumentBoundary:
        // FIXME: Implement all of the above.
        pos = ModifyMovingForward(granularity, reachedBoundary);
        break;
      case TextGranularity::LineBoundary:
        pos = RightBoundaryOfLine(StartForPlatform(), DirectionOfEnclosingBlock(), reachedBoundary);
        break;
      case TextGranularity::DocumentGranularity: krys_debug_assert(false); break;
    }
    return pos;
  }

  VisiblePosition FrameSelection::ModifyMovingForward(TextGranularity granularity,
                                                      bool *reachedBoundary) noexcept
  {
    if (reachedBoundary)
    {
      *reachedBoundary = false;
    }
    VisiblePosition currentPosition;
    switch (granularity)
    {
      case TextGranularity::WordGranularity:
      case TextGranularity::SentenceGranularity:
        currentPosition = VisiblePosition(_selection.Extent(), _selection.GetAffinity());
        break;
      case TextGranularity::LineGranularity:
      case TextGranularity::ParagraphGranularity:
      case TextGranularity::SentenceBoundary:
      case TextGranularity::ParagraphBoundary:
      case TextGranularity::DocumentBoundary:     currentPosition = EndForPlatform(); break;
      default:                                    break;
    }
    VisiblePosition pos;
    // FIXME: Stay in editable content for the less common granularities.
    switch (granularity)
    {
      case TextGranularity::CharacterGranularity:
        if (IsRange())
        {
          pos = VisiblePosition(_selection.End(), _selection.GetAffinity());
        }
        else
        {
          pos = VisiblePosition(_selection.Extent(), _selection.GetAffinity())
                  .Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary, reachedBoundary);
        }
        break;
      case TextGranularity::WordGranularity:     pos = NextWordPositionForPlatform(currentPosition); break;
      case TextGranularity::SentenceGranularity: pos = NextSentencePosition(currentPosition); break;
      case TextGranularity::LineGranularity:
      {
        // down-arrowing from a range selection that ends at the start of a line needs
        // to leave the selection at that line start (no need to call NextLinePosition!)
        pos = currentPosition;
        if (!IsRange() || !IsStartOfLine(pos))
        {
          pos = NextLinePosition(pos, LineDirectionPointForBlockDirectionNavigation(PositionType::Start));
        }
        break;
      }
      case TextGranularity::ParagraphGranularity:
        pos = NextParagraphPosition(currentPosition,
                                    LineDirectionPointForBlockDirectionNavigation(PositionType::Start));
        break;
      case TextGranularity::DocumentGranularity: krys_debug_assert(false); break;
      case TextGranularity::SentenceBoundary:    pos = EndOfSentence(currentPosition); break;
      case TextGranularity::LineBoundary:        pos = LogicalEndOfLine(EndForPlatform(), reachedBoundary); break;
      case TextGranularity::ParagraphBoundary:   pos = EndOfParagraph(currentPosition); break;
      case TextGranularity::DocumentBoundary:
        pos = currentPosition;
        if (IsEditablePosition(pos.DeepEquivalent()))
        {
          pos = EndOfEditableContent(pos);
        }
        else
        {
          pos = EndOfDocument(pos);
        }
        break;
    }
    switch (granularity)
    {
      case TextGranularity::WordGranularity:
      case TextGranularity::SentenceGranularity:
      case TextGranularity::LineGranularity:
      case TextGranularity::ParagraphGranularity:
      case TextGranularity::SentenceBoundary:
      case TextGranularity::ParagraphBoundary:
      case TextGranularity::DocumentBoundary:
        if (reachedBoundary)
        {
          *reachedBoundary = pos == currentPosition;
        }
        break;
      default: break;
    }
    return pos;
  }

  VisiblePosition FrameSelection::ModifyExtendingLeft(TextGranularity granularity,
                                                      UserTriggered userTriggered) noexcept
  {
    VisiblePosition pos(_selection.Extent(), _selection.GetAffinity());

    // The difference between ModifyExtendingLeft and ModifyExtendingBackward is:
    // ModifyExtendingBackward always extends backward logically.
    // ModifyExtendingLeft behaves the same as ModifyExtendingBackward except for extending character or word,
    // it extends backward logically if the enclosing block is TextDirection::LTR,
    // but it extends forward logically if the enclosing block is TextDirection::RTL.
    switch (granularity)
    {
      case TextGranularity::CharacterGranularity:
        if (DirectionOfEnclosingBlock() == TextDirection::LTR)
        {
          pos = pos.Previous(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
        }
        else
        {
          pos = pos.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
        }
        break;
      case TextGranularity::WordGranularity:
        if (DirectionOfEnclosingBlock() == TextDirection::LTR)
        {
          pos = PreviousWordPosition(pos);
        }
        else
        {
          pos = NextWordPositionForPlatform(pos);
        }
        break;
      case TextGranularity::LineBoundary:
        if (DirectionOfEnclosingBlock() == TextDirection::LTR)
        {
          pos = ModifyExtendingBackward(granularity, userTriggered);
        }
        else
        {
          pos = ModifyExtendingForward(granularity, userTriggered);
        }
        break;
      case TextGranularity::SentenceGranularity:
      case TextGranularity::LineGranularity:
      case TextGranularity::ParagraphGranularity:
      case TextGranularity::SentenceBoundary:
      case TextGranularity::ParagraphBoundary:
      case TextGranularity::DocumentBoundary:
        pos = ModifyExtendingBackward(granularity, userTriggered);
        break;
      case TextGranularity::DocumentGranularity: krys_debug_assert(false); break;
    }
    AdjustSelectionExtentIfNeeded(pos, DirectionOfEnclosingBlock() == TextDirection::RTL, userTriggered);
    return pos;
  }

  VisiblePosition FrameSelection::ModifyExtendingBackward(TextGranularity granularity,
                                                          UserTriggered userTriggered) noexcept
  {
    VisiblePosition pos(_selection.Extent(), _selection.GetAffinity());

    // Extending a selection backward by word or character from just after a table selects
    // the table.  This "makes sense" from the user perspective, esp. when deleting.
    // It was done here instead of in VisiblePosition because we want VPs to iterate
    // over everything.
    switch (granularity)
    {
      case TextGranularity::CharacterGranularity:
        pos = pos.Previous(EditingBoundaryCrossingRule::CannotCrossEditingBoundary);
        break;
      case TextGranularity::WordGranularity:     pos = PreviousWordPosition(pos); break;
      case TextGranularity::SentenceGranularity: pos = PreviousSentencePosition(pos); break;
      case TextGranularity::LineGranularity:
        pos = PreviousLinePosition(pos, LineDirectionPointForBlockDirectionNavigation(PositionType::Extent));
        break;
      case TextGranularity::ParagraphGranularity:
        pos =
          PreviousParagraphPosition(pos, LineDirectionPointForBlockDirectionNavigation(PositionType::Extent));
        break;
      case TextGranularity::SentenceBoundary:  pos = StartOfSentence(StartForPlatform()); break;
      case TextGranularity::LineBoundary:      pos = LogicalStartOfLine(StartForPlatform()); break;
      case TextGranularity::ParagraphBoundary: pos = StartOfParagraph(StartForPlatform()); break;
      case TextGranularity::DocumentBoundary:
        pos = StartForPlatform();
        if (IsEditablePosition(pos.DeepEquivalent()))
        {
          pos = StartOfEditableContent(pos);
        }
        else
        {
          pos = StartOfDocument(pos);
        }
        break;
      case TextGranularity::DocumentGranularity: krys_debug_assert(false); break;
    }
    AdjustSelectionExtentIfNeeded(pos, DirectionOfEnclosingBlock() == TextDirection::RTL, userTriggered);
    return pos;
  }

  VisiblePosition FrameSelection::ModifyMovingLeft(TextGranularity granularity,
                                                   bool *reachedBoundary) noexcept
  {
    if (reachedBoundary)
    {
      *reachedBoundary = false;
    }
    VisiblePosition pos;
    switch (granularity)
    {
      case TextGranularity::CharacterGranularity:
        if (IsRange())
        {
          if (DirectionOfSelection() == TextDirection::LTR)
          {
            pos = VisiblePosition(_selection.Start(), _selection.GetAffinity());
          }
          else
          {
            pos = VisiblePosition(_selection.End(), _selection.GetAffinity());
          }
        }
        else
        {
          pos = VisiblePosition(_selection.Extent(), _selection.GetAffinity()).Left(true, reachedBoundary);
        }
        break;
      case TextGranularity::WordGranularity:
      {
        bool skipsSpaceWhenMovingRight = Behavior(_document).ShouldSkipSpaceWhenMovingRight();
        VisiblePosition currentPosition(_selection.Extent(), _selection.GetAffinity());
        pos = LeftWordPosition(currentPosition, skipsSpaceWhenMovingRight);
        if (reachedBoundary)
        {
          *reachedBoundary = pos == currentPosition;
        }
        break;
      }
      case TextGranularity::SentenceGranularity:
      case TextGranularity::LineGranularity:
      case TextGranularity::ParagraphGranularity:
      case TextGranularity::SentenceBoundary:
      case TextGranularity::ParagraphBoundary:
      case TextGranularity::DocumentBoundary:
        // FIXME: Implement all of the above.
        pos = ModifyMovingBackward(granularity, reachedBoundary);
        break;
      case TextGranularity::LineBoundary:
        pos = LeftBoundaryOfLine(StartForPlatform(), DirectionOfEnclosingBlock(), reachedBoundary);
        break;
      case TextGranularity::DocumentGranularity: krys_debug_assert(false); break;
    }
    return pos;
  }

  VisiblePosition FrameSelection::ModifyMovingBackward(TextGranularity granularity,
                                                       bool *reachedBoundary) noexcept
  {
    if (reachedBoundary)
    {
      *reachedBoundary = false;
    }
    VisiblePosition currentPosition;
    switch (granularity)
    {
      case TextGranularity::WordGranularity:
      case TextGranularity::SentenceGranularity:
        currentPosition = VisiblePosition(_selection.Extent(), _selection.GetAffinity());
        break;
      case TextGranularity::LineGranularity:
      case TextGranularity::ParagraphGranularity:
      case TextGranularity::SentenceBoundary:
      case TextGranularity::ParagraphBoundary:
      case TextGranularity::DocumentBoundary:     currentPosition = StartForPlatform(); break;
      default:                                    break;
    }
    VisiblePosition pos;
    switch (granularity)
    {
      case TextGranularity::CharacterGranularity:
        if (IsRange())
        {
          pos = VisiblePosition(_selection.Start(), _selection.GetAffinity());
        }
        else
        {
          pos = VisiblePosition(_selection.Extent(), _selection.GetAffinity())
                  .Previous(EditingBoundaryCrossingRule::CannotCrossEditingBoundary, reachedBoundary);
        }
        break;
      case TextGranularity::WordGranularity:     pos = PreviousWordPosition(currentPosition); break;
      case TextGranularity::SentenceGranularity: pos = PreviousSentencePosition(currentPosition); break;
      case TextGranularity::LineGranularity:
        pos = PreviousLinePosition(currentPosition,
                                   LineDirectionPointForBlockDirectionNavigation(PositionType::Start));
        break;
      case TextGranularity::ParagraphGranularity:
        pos = PreviousParagraphPosition(currentPosition,
                                        LineDirectionPointForBlockDirectionNavigation(PositionType::Start));
        break;
      case TextGranularity::SentenceBoundary: pos = StartOfSentence(currentPosition); break;
      case TextGranularity::LineBoundary:
        pos = LogicalStartOfLine(StartForPlatform(), reachedBoundary);
        break;
      case TextGranularity::ParagraphBoundary: pos = StartOfParagraph(currentPosition); break;
      case TextGranularity::DocumentBoundary:
        pos = currentPosition;
        if (IsEditablePosition(pos.DeepEquivalent()))
        {
          pos = StartOfEditableContent(pos);
        }
        else
        {
          pos = StartOfDocument(pos);
        }
        break;
      case TextGranularity::DocumentGranularity: krys_debug_assert(false); break;
    }
    switch (granularity)
    {
      case TextGranularity::WordGranularity:
      case TextGranularity::SentenceGranularity:
      case TextGranularity::LineGranularity:
      case TextGranularity::ParagraphGranularity:
      case TextGranularity::SentenceBoundary:
      case TextGranularity::ParagraphBoundary:
      case TextGranularity::DocumentBoundary:
        if (reachedBoundary)
        {
          *reachedBoundary = pos == currentPosition;
        }
        break;
      default: break;
    }
    return pos;
  }

  // FIXME: Accessibility's text selection intents and boundary notifications, with accessibility.
  bool FrameSelection::Modify(Alteration alter, SelectionDirection direction, TextGranularity granularity,
                              UserTriggered userTriggered) noexcept
  {
    if (userTriggered == UserTriggered::Yes)
    {
      FrameSelection trialFrameSelection;
      trialFrameSelection.SetSelection(_selection);
      (void)trialFrameSelection.Modify(alter, direction, granularity, UserTriggered::No);

      bool change = ShouldChangeSelection(trialFrameSelection.Selection());
      if (!change)
      {
        return false;
      }

      if (trialFrameSelection.Selection().IsRange() && _selection.IsCaret() && !DispatchSelectStart())
      {
        return false;
      }
    }

    WillBeModified(alter, direction);

    // Before modifying selection, update layout and disable post resolution callbacks.
    // That way, unaverted tree changes are avoided while browsing the document.
    auto selectionDocument = _selection.GetDocument();
    if (!selectionDocument)
    {
      return false;
    }
    (void)selectionDocument->UpdateLayoutIgnorePendingStylesheets();
    style::PostResolutionCallbackDisabler disabler(*selectionDocument);

    bool reachedBoundary = false;
    bool wasRange = _selection.IsRange();
    dom::Position originalStartPosition = _selection.Start();
    VisiblePosition position;
    switch (direction)
    {
      case SelectionDirection::Right:
        if (alter == Alteration::Move)
        {
          position = ModifyMovingRight(granularity, &reachedBoundary);
        }
        else
        {
          position = ModifyExtendingRight(granularity, userTriggered);
        }
        break;
      case SelectionDirection::Forward:
        if (alter == Alteration::Extend)
        {
          position = ModifyExtendingForward(granularity, userTriggered);
        }
        else
        {
          position = ModifyMovingForward(granularity, &reachedBoundary);
        }
        break;
      case SelectionDirection::Left:
        if (alter == Alteration::Move)
        {
          position = ModifyMovingLeft(granularity, &reachedBoundary);
        }
        else
        {
          position = ModifyExtendingLeft(granularity, userTriggered);
        }
        break;
      case SelectionDirection::Backward:
        if (alter == Alteration::Extend)
        {
          position = ModifyExtendingBackward(granularity, userTriggered);
        }
        else
        {
          position = ModifyMovingBackward(granularity, &reachedBoundary);
        }
        break;
    }

    if (position.IsNull())
    {
      return false;
    }

    if (_document && _document->GetSettings().SpatialNavigationEnabled)
    {
      if (!wasRange && alter == Alteration::Move && position == VisiblePosition(originalStartPosition))
      {
        return false;
      }
    }

    // Some of the above operations set an xPosForVerticalArrowNavigation.
    // Setting a selection will clear it, so save it to possibly restore later.
    // Note: the Start position type is arbitrary because it is unused, it would be
    // the requested position type if there were no xPosForVerticalArrowNavigation set.
    LayoutUnit x = LineDirectionPointForBlockDirectionNavigation(PositionType::Start);

    _selection.SetDirectionality(
      (ShouldAlwaysUseDirectionalSelection(_document) || alter == Alteration::Extend) ? Directionality::Strong
                                                                                      : Directionality::None);

    switch (alter)
    {
      case Alteration::Move: MoveTo(position, userTriggered); break;
      case Alteration::Extend:
      {
        auto behavior = Behavior(_document);
        if (!_selection.IsCaret()
            && (granularity == TextGranularity::WordGranularity
                || granularity == TextGranularity::ParagraphGranularity
                || granularity == TextGranularity::LineGranularity)
            && _document && !behavior.ShouldExtendSelectionByWordOrLineAcrossCaret())
        {
          // Don't let the selection go across the base position directly. Needed to match mac
          // behavior when, for instance, word-selecting backwards starting with the caret in
          // the middle of a word and then word-selecting forward, leaving the caret in the
          // same place where it was, instead of directly selecting to the end of the word.
          VisibleSelection newSelection = _selection;
          newSelection.SetExtent(position);
          if (_selection.IsBaseFirst() != newSelection.IsBaseFirst())
          {
            position = VisiblePosition(_selection.Base());
          }
        }

        // Standard Mac behavior when extending to a boundary is grow the selection rather than leaving the
        // base in place and moving the extent. Matches NSTextView.
        if (!_document || !behavior.ShouldAlwaysGrowSelectionWhenExtendingToBoundary() || _selection.IsCaret()
            || !IsBoundary(granularity))
        {
          SetExtent(position, userTriggered);
        }
        else
        {
          TextDirection textDirection = DirectionOfEnclosingBlock();
          if (direction == SelectionDirection::Forward
              || (textDirection == TextDirection::LTR && direction == SelectionDirection::Right)
              || (textDirection == TextDirection::RTL && direction == SelectionDirection::Left))
          {
            SetEnd(position, userTriggered);
          }
          else
          {
            SetStart(position, userTriggered);
          }
        }
        break;
      }
    }

    if (granularity == TextGranularity::LineGranularity
        || granularity == TextGranularity::ParagraphGranularity)
    {
      _xPosForVerticalArrowNavigation = x;
    }

    if (userTriggered == UserTriggered::Yes)
    {
      _granularity = TextGranularity::CharacterGranularity;
    }

    SetCaretRectNeedsUpdate();

    (void)reachedBoundary;
    return true;
  }

  bool FrameSelection::Modify(Alteration alter, uint32 verticalDistance, VerticalDirection direction,
                              UserTriggered userTriggered, CursorAlignOnScroll align) noexcept
  {
    if (!verticalDistance)
    {
      return false;
    }

    if (userTriggered == UserTriggered::Yes)
    {
      FrameSelection trialFrameSelection;
      trialFrameSelection.SetSelection(_selection);
      (void)trialFrameSelection.Modify(alter, verticalDistance, direction, UserTriggered::No);

      bool change = ShouldChangeSelection(trialFrameSelection.Selection());
      if (!change)
      {
        return false;
      }
    }

    WillBeModified(alter, direction == VerticalDirection::Up ? SelectionDirection::Backward
                                                             : SelectionDirection::Forward);

    VisiblePosition pos;
    LayoutUnit xPos;
    switch (alter)
    {
      case Alteration::Move:
        pos = VisiblePosition(direction == VerticalDirection::Up ? _selection.Start() : _selection.End(),
                              _selection.GetAffinity());
        xPos = LineDirectionPointForBlockDirectionNavigation(
          direction == VerticalDirection::Up ? PositionType::Start : PositionType::End);
        _selection.SetAffinity(direction == VerticalDirection::Up ? Affinity::Upstream
                                                                  : Affinity::Downstream);
        break;
      case Alteration::Extend:
        pos = VisiblePosition(_selection.Extent(), _selection.GetAffinity());
        xPos = LineDirectionPointForBlockDirectionNavigation(PositionType::Extent);
        _selection.SetAffinity(Affinity::Downstream);
        break;
    }

    int32 startY = 0;
    if (!AbsoluteCaretY(pos, startY))
    {
      return false;
    }
    if (direction == VerticalDirection::Up)
    {
      startY = -startY;
    }
    int32 lastY = startY;

    VisiblePosition result;
    VisiblePosition next;
    for (VisiblePosition p = pos;; p = next)
    {
      if (direction == VerticalDirection::Up)
      {
        next = PreviousLinePosition(p, xPos);
      }
      else
      {
        next = NextLinePosition(p, xPos);
      }

      if (next.IsNull() || next == p)
      {
        break;
      }
      int32 nextY = 0;
      if (!AbsoluteCaretY(next, nextY))
      {
        break;
      }
      if (direction == VerticalDirection::Up)
      {
        nextY = -nextY;
      }
      if (nextY - startY > static_cast<int32>(verticalDistance))
      {
        break;
      }
      if (nextY >= lastY)
      {
        lastY = nextY;
        result = next;
      }
    }

    if (result.IsNull())
    {
      return false;
    }

    switch (alter)
    {
      case Alteration::Move:   MoveTo(result, userTriggered, align); break;
      case Alteration::Extend: SetExtent(result, userTriggered); break;
    }

    if (userTriggered == UserTriggered::Yes)
    {
      _granularity = TextGranularity::CharacterGranularity;
    }

    _selection.SetDirectionality(
      (ShouldAlwaysUseDirectionalSelection(_document) || alter == Alteration::Extend) ? Directionality::Strong
                                                                                      : Directionality::None);
    return true;
  }

  LayoutUnit FrameSelection::LineDirectionPointForBlockDirectionNavigation(PositionType type) noexcept
  {
    if (IsNone())
    {
      return LayoutUnit(0);
    }

    // FIXME: Can we use VisibleStart/End/Extent?
    dom::Position position;
    switch (type)
    {
      case PositionType::Start:  position = _selection.Start(); break;
      case PositionType::End:    position = _selection.End(); break;
      case PositionType::Extent: position = _selection.Extent(); break;
    }

    // FIXME: Why is this check needed? What's the harm in doing a little more work without a frame?
    if (!position.AnchorNode()->NodeDocument().Frame())
    {
      return LayoutUnit(0);
    }

    // FIXME: Can we do this before getting the position from the selection?
    if (_xPosForVerticalArrowNavigation)
    {
      return *_xPosForVerticalArrowNavigation;
    }

    // VisiblePosition creation can fail here if a node containing the selection becomes
    // visibility:hidden after the selection is created and before this function is called.
    VisiblePosition visiblePosition(position, _selection.GetAffinity());
    auto x = LayoutUnit(
      visiblePosition.IsNotNull() ? visiblePosition.LineDirectionPointForBlockDirectionNavigation() : 0);
    _xPosForVerticalArrowNavigation = x;
    return x;
  }

  bool FrameSelection::DispatchSelectStart() noexcept
  {
    RefPtr<dom::Node> selectStartTarget = ShareRefPtr(_selection.Extent().GetContainerNode());
    if (!selectStartTarget)
    {
      return true;
    }

    auto type = dom::DOMStringAtom(EventNames::SelectStart);
    return dom::EventDispatcher::FireEvent<dom::Event>(type, *selectStartTarget, [&](dom::Event &event)
                                                       { event.InitEvent(type, true, true); });
  }

  void FrameSelection::SetStart(const VisiblePosition &position, UserTriggered trigger) noexcept
  {
    if (_selection.IsBaseFirst())
    {
      SetBase(position, trigger);
    }
    else
    {
      SetExtent(position, trigger);
    }
  }

  void FrameSelection::SetEnd(const VisiblePosition &position, UserTriggered trigger) noexcept
  {
    if (_selection.IsBaseFirst())
    {
      SetExtent(position, trigger);
    }
    else
    {
      SetBase(position, trigger);
    }
  }

  void FrameSelection::SetBase(const VisiblePosition &position, UserTriggered userTriggered) noexcept
  {
    SetSelection(VisibleSelection(position.DeepEquivalent(), _selection.Extent(), position.GetAffinity(),
                                  Directionality::Strong),
                 DefaultSetSelectionOptions(userTriggered));
  }

  void FrameSelection::SetExtent(const VisiblePosition &position, UserTriggered userTriggered) noexcept
  {
    SetSelection(VisibleSelection(_selection.Base(), position.DeepEquivalent(), position.GetAffinity(),
                                  Directionality::Strong),
                 DefaultSetSelectionOptions(userTriggered));
  }

  void FrameSelection::SetBase(const dom::Position &position, Affinity affinity,
                               UserTriggered userTriggered) noexcept
  {
    SetSelection(VisibleSelection(position, _selection.Extent(), affinity, Directionality::Strong),
                 DefaultSetSelectionOptions(userTriggered));
  }

  void FrameSelection::SetExtent(const dom::Position &position, Affinity affinity,
                                 UserTriggered userTriggered) noexcept
  {
    SetSelection(VisibleSelection(_selection.Base(), position, affinity, Directionality::Strong),
                 DefaultSetSelectionOptions(userTriggered));
  }

  // FIXME: Image overlays (ImageOverlay::isInsideOverlay), once there are any.
  bool FrameSelection::Contains(const LayoutPoint &point) const noexcept
  {
    // Treat a collapsed selection like no selection.
    if (!IsRange())
    {
      return false;
    }

    auto range = _selection.FirstRange();
    if (!range)
    {
      return false;
    }

    auto *document = _document;
    if (!document)
    {
      return false;
    }

    HitTestResult result(point);
    document->HitTest(HitTestRequest(), result);
    auto *innerNode = result.InnerNode();
    if (!innerNode || !innerNode->Renderer())
    {
      return false;
    }

    return dom::Contains(*range, MakeBoundaryPoint(innerNode->Renderer()->VisiblePositionForPoint(
                                   result.LocalPoint(), HitTestSource::User)));
  }

  // FIXME: Select elements (selectAll on a focused select), with them.
  void FrameSelection::SelectAll() noexcept
  {
    auto *focusedElement = _document->FocusedElement();

    RefPtr<dom::Node> root;
    RefPtr<dom::Node> selectStartTarget;
    if (_selection.IsContentEditable())
    {
      root = HighestEditableRoot(_selection.Start());
      if (auto *shadowRoot = _selection.NonBoundaryShadowTreeRootNode())
      {
        selectStartTarget = ShareRefPtr<dom::Node>(dom::TreeQueries::ShadowHost(*shadowRoot));
      }
      else
      {
        selectStartTarget = root;
      }
    }
    else
    {
      if (_selection.IsNone() && focusedElement)
      {
        if (focusedElement->IsTextField())
        {
          Downcast<html::HTMLTextFormControlElement>(*focusedElement).Select();
          return;
        }
        root = ShareRefPtr(focusedElement->NonBoundaryShadowTreeRootNode());
      }
      else
      {
        root = ShareRefPtr(_selection.NonBoundaryShadowTreeRootNode());
      }

      if (root)
      {
        selectStartTarget = ShareRefPtr<dom::Node>(dom::TreeQueries::ShadowHost(*root));
      }
      else
      {
        root = _document->DocumentElement();
        selectStartTarget = _document->Body();
      }
    }
    if (!root)
    {
      return;
    }

    if (selectStartTarget)
    {
      auto type = dom::DOMStringAtom(EventNames::SelectStart);
      bool notCanceled = dom::EventDispatcher::FireEvent<dom::Event>(
        type, *selectStartTarget, [&](dom::Event &event) { event.InitEvent(type, true, true); });
      if (!notCanceled)
      {
        return;
      }
    }

    VisibleSelection newSelection(VisibleSelection::SelectionFromContentsOfNode(root.get()));
    if (!newSelection.IsOrphan() && ShouldChangeSelection(newSelection))
    {
      auto options = DefaultSetSelectionOptions();
      options.Add(SetSelectionOption::FireSelectEvent);
      SetSelection(newSelection, options);
    }
  }

  bool FrameSelection::SetSelectedRange(const Maybe<dom::SimpleRange> &range, Affinity affinity,
                                        ShouldCloseTyping closeTyping, UserTriggered userTriggered) noexcept
  {
    if (!range)
    {
      return false;
    }

    if (&range->Start.GetDocument() != &range->End.GetDocument())
    {
      return false;
    }

    VisibleSelection newSelection(*range, affinity);

    EnumSet<SetSelectionOption> selectionOptions {SetSelectionOption::ClearTypingStyle};
    if (closeTyping == ShouldCloseTyping::Yes)
    {
      selectionOptions.Add(SetSelectionOption::CloseTyping);
    }

    if (userTriggered == UserTriggered::Yes)
    {
      FrameSelection trialFrameSelection;

      trialFrameSelection.SetSelection(newSelection, selectionOptions);

      if (!ShouldChangeSelection(trialFrameSelection.Selection()))
      {
        return false;
      }

      selectionOptions.Add(SetSelectionOption::IsUserTriggered);
    }

    SetSelection(newSelection, selectionOptions);
    return true;
  }

  void FrameSelection::FocusedOrActiveStateChanged() noexcept
  {
    RefPtr<dom::Document> document = ShareRefPtr(_document);
    document->UpdateStyleIfNeeded();

    // Because RenderObject::SelectionBackgroundColor() and
    // RenderObject::SelectionForegroundColor() check if the frame is active,
    // we have to update places those colors were painted.
    if (auto *view = document->GetRenderView())
    {
      view->Selection().Repaint();
    }

    // Caret appears in the active frame.
    if (IsFocusedAndActive())
    {
      SetSelectionFromNone();
      RemoveCaretVisibilitySuppressionReason(CaretVisibilitySuppressionReason::IsNotFocusedOrActive);
    }
    else
    {
      AddCaretVisibilitySuppressionReason(CaretVisibilitySuppressionReason::IsNotFocusedOrActive);
    }
  }

  void FrameSelection::SetFocused(bool isFocused) noexcept
  {
    if (_focused == isFocused)
    {
      return;
    }

    _focused = isFocused;

    FocusedOrActiveStateChanged();
  }

  bool FrameSelection::IsFocusedAndActive() const noexcept
  {
    return _focused;
  }

  // FIXME: Stopping the blinking while typing (shouldStopBlinkingDueToTypingCommand), with editing commands.
  void FrameSelection::UpdateAppearance() noexcept
  {
    VisibleSelection oldSelection = Selection();

    auto *document = _document;
    bool caretRectChangedOrCleared = RecomputeCaretRect();

    bool caretBrowsing = document->GetSettings().CaretBrowsingEnabled;
    bool shouldBlink = CaretIsVisible() && IsCaret() && (oldSelection.IsContentEditable() || caretBrowsing);

    // If the caret moved, stop the blink timer so we can restart with a
    // black caret in the new location.
    if (caretRectChangedOrCleared || !shouldBlink)
    {
      GetCaretAnimator().Stop(CaretAnimatorStopReason::CaretRectChanged);
    }

    // Start blinking with a black caret. Be sure not to restart if we're
    // already blinking in the right location.
    if (shouldBlink && !GetCaretAnimator().IsActive())
    {
      if (document && document->Frame())
      {
        GetCaretAnimator().Start();
      }

      GetCaretAnimator().SetVisible(true);
    }

    // Construct a new VisibleSolution, since _selection is not necessarily valid, and the following steps
    // assume a valid selection.
    VisibleSelection selection(oldSelection.VisibleStart(), oldSelection.VisibleEnd());

    auto *view = document->GetRenderView();
    if (!view)
    {
      return;
    }
    if (!selection.IsRange())
    {
      view->Selection().Clear();
      return;
    }

    // Use the rightmost candidate for the start of the selection, and the leftmost candidate for the end of
    // the selection. Example: foo <a>bar</a>.  Imagine that a line wrap occurs after 'foo', and that 'bar' is
    // selected.   If we pass [foo, 3] as the start of the selection, the selection painting code will think
    // that content on the line containing 'foo' is selected and will fill the gap before 'bar'.
    dom::Position startPos = selection.Start();
    dom::Position candidate = startPos.Downstream();
    if (candidate.IsCandidate())
    {
      startPos = candidate;
    }
    dom::Position endPos = selection.End();
    candidate = endPos.Upstream();
    if (candidate.IsCandidate())
    {
      endPos = candidate;
    }

    // We can get into a state where the selection endpoints map to the same VisiblePosition when a selection
    // is deleted because we don't yet notify the FrameSelection of text removal.
    if (startPos.IsNotNull() && endPos.IsNotNull() && selection.VisibleStart() != selection.VisibleEnd())
    {
      auto *startRenderer = startPos.DeprecatedNode()->Renderer();
      int32 startOffset = startPos.DeprecatedEditingOffset();
      auto *endRenderer = endPos.DeprecatedNode()->Renderer();
      int32 endOffset = endPos.DeprecatedEditingOffset();
      krys_debug_assert(startOffset >= 0 && endOffset >= 0);
      view->Selection().Set(
        {startRenderer, endRenderer, static_cast<uint32>(startOffset), static_cast<uint32>(endOffset)});
    }
  }

  bool FrameSelection::ShouldChangeSelection(const VisibleSelection &) const noexcept
  {
    return true;
  }

  // FIXME: Asking the editor client whether to delete the range (shouldDeleteRange), with the editing
  // delegates.
  bool FrameSelection::ShouldDeleteSelection(const VisibleSelection &) const noexcept
  {
    return true;
  }

  FloatRect FrameSelection::SelectionBounds(ClipToVisibleContent clipToVisibleContent) noexcept
  {
    if (!_document)
    {
      return {};
    }

    UpdateSelectionAppearanceNow();
    auto *renderView = _document->GetRenderView();
    if (!renderView)
    {
      return {};
    }

    if (!_selection.Range())
    {
      return {};
    }

    const auto &selection = renderView->Selection();
    auto visibleSelectionRect = selection.BoundsClippedToVisibleContent();

    if (clipToVisibleContent == ClipToVisibleContent::No)
    {
      return FloatRect(selection.Bounds());
    }

    return Intersection(FloatRect(visibleSelectionRect),
                        FloatRect(renderView->FrameView().VisibleContentRect()));
  }

  // FIXME: Caret browsing, once there's caret browsing.
  void FrameSelection::SetSelectionFromNone() noexcept
  {
    // Put a caret inside the body if the entire frame is editable (either the
    // entire WebView is editable or designMode is on for this document).
    if (!_document || !IsNone() || !_document->HasEditableStyle())
    {
      return;
    }

    if (auto body = _document->Body())
    {
      SetSelection(
        VisibleSelection(FirstPositionInOrBeforeNode(body.get()), VisibleSelection::DefaultAffinity));
    }
  }

  void FrameSelection::UpdateAppearanceAfterUpdatingRendering() noexcept
  {
    SetCaretRectNeedsUpdate();
    UpdateAndRevealSelection();
  }

  CaretBase::CaretBase(CaretVisibility visibility) noexcept : _caretVisibility(visibility)
  {
  }

  void CaretBase::ClearCaretRect() noexcept
  {
    _caretLocalRect = LayoutRect();
  }

  bool CaretBase::UpdateCaretRect(dom::Document &document, const VisiblePosition &caretPosition) noexcept
  {
    (void)document.UpdateLayoutIgnorePendingStylesheets();
    _caretRectNeedsUpdate = false;
    RenderBlock *renderer = nullptr;
    _caretLocalRect = LocalCaretRectInRendererForCaretPainting(caretPosition, renderer);
    return !_caretLocalRect.IsEmpty();
  }

  RenderBlock *FrameSelection::CaretRendererWithoutUpdatingLayout() const noexcept
  {
    return RendererForCaretPainting(_selection.Start().DeprecatedNode());
  }

  IntRect FrameSelection::AbsoluteCaretBounds(bool *insideFixed) noexcept
  {
    if (!_document)
    {
      return IntRect();
    }
    UpdateSelectionAppearanceNow();
    (void)RecomputeCaretRect();
    if (insideFixed)
    {
      *insideFixed = _caretInsidePositionFixed;
    }
    return _absCaretBounds;
  }

  bool FrameSelection::RecomputeCaretRect() noexcept
  {
    if (!ShouldUpdateCaretRect())
    {
      return false;
    }

    RefPtr<dom::Document> document = ShareRefPtr(_document);
    if (!document)
    {
      return false;
    }

    if (!document->View())
    {
      return false;
    }

    LayoutRect oldRect = LocalCaretRectWithoutUpdate();

    RefPtr<dom::Node> caretNode = _previousCaretNode;
    if (ShouldUpdateCaretRect())
    {
      if (!IsNonOrphanedCaret(_selection))
      {
        ClearCaretRect();
      }
      else
      {
        VisiblePosition visibleStart = _selection.VisibleStart();
        if (UpdateCaretRect(*document, visibleStart))
        {
          caretNode = ShareRefPtr(visibleStart.DeepEquivalent().DeprecatedNode());
          _absCaretBoundsDirty = true;
        }
      }
    }
    LayoutRect newRect = LocalCaretRectWithoutUpdate();

    if (caretNode == _previousCaretNode && oldRect == newRect && !_absCaretBoundsDirty)
    {
      return false;
    }

    IntRect oldAbsCaretBounds = _absCaretBounds;
    bool isInsideFixed = false;
    _absCaretBounds =
      AbsoluteBoundsForLocalCaretRect(RendererForCaretPainting(caretNode.get()), newRect, &isInsideFixed);
    _caretInsidePositionFixed = isInsideFixed;

    _absCaretBoundsDirty = false;

    if (caretNode == _previousCaretNode && oldAbsCaretBounds == _absCaretBounds)
    {
      return false;
    }

    if (auto *view = document->GetRenderView())
    {
      bool previousOrNewCaretNodeIsContentEditable =
        _selection.IsContentEditable() || (_previousCaretNode && _previousCaretNode->IsContentEditable());
      if (ShouldRepaintCaret(view, previousOrNewCaretNodeIsContentEditable))
      {
        if (_previousCaretNode)
        {
          RepaintCaretForLocalRect(_previousCaretNode.get(), oldRect, _caretAnimator.get());
        }
        _previousCaretNode = caretNode;
        RepaintCaretForLocalRect(caretNode.get(), newRect, _caretAnimator.get());
      }
    }
    return true;
  }

  bool CaretBase::ShouldRepaintCaret(const RenderView *view, bool isContentEditable) const noexcept
  {
    krys_debug_assert(view);
    bool caretBrowsing =
      view->GetDocument().GetSettings().CaretBrowsingEnabled; // The frame where the selection started.
    return (caretBrowsing || isContentEditable);
  }

  void FrameSelection::InvalidateCaretRect() noexcept
  {
    if (!IsCaret())
    {
      return;
    }

    CaretBase::InvalidateCaretRect(_selection.Start().DeprecatedNode(), RecomputeCaretRect(),
                                   _caretAnimator.get());
  }

  void CaretBase::InvalidateCaretRect(dom::Node *node, bool caretRectChanged,
                                      CaretAnimator *caretAnimator) noexcept
  {
    // EDIT FIXME: This is an unfortunate hack.
    // Basically, we can't trust this layout position since we
    // can't guarantee that the check to see if we are in unrendered
    // content will work at this point. We may have to wait for
    // a layout and re-render of the document to happen. So, resetting this
    // flag will cause another caret layout to happen the first time
    // that we try to paint the caret after this call. That one will work since
    // it happens after the document has accounted for any editing
    // changes which may have been done.
    // And, we need to leave this layout here so the caret moves right
    // away after clicking.
    _caretRectNeedsUpdate = true;

    if (caretRectChanged)
    {
      return;
    }

    if (auto *view = node->NodeDocument().GetRenderView())
    {
      if (ShouldRepaintCaret(view, IsEditableNode(*node)))
      {
        RepaintCaretForLocalRect(node, LocalCaretRectWithoutUpdate(), caretAnimator);
      }
    }
  }

  void FrameSelection::PaintCaret(GraphicsContext &context, const LayoutPoint &paintOffset) noexcept
  {
    if (_selection.IsCaret() && _selection.Start().DeprecatedNode())
    {
      CaretBase::PaintCaret(*_selection.Start().DeprecatedNode(), context, paintOffset, _caretAnimator.get());
    }
  }

  Color CaretBase::ComputeCaretColor(const RenderStyle &elementStyle, const dom::Node *node) noexcept
  {
    auto *parentElement = node ? node->ParentElement() : nullptr;
    const RenderStyle *parentStyle =
      parentElement && parentElement->Renderer() ? &parentElement->Renderer()->Style() : nullptr;
    // CSS value "auto" is treated as an invalid color.
    if (elementStyle.HasAutoCaretColor() && parentStyle)
    {
      auto parentBackgroundColor =
        parentStyle->VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor);
      auto elementBackgroundColor =
        elementStyle.VisitedDependentColorWithColorFilter(css::PropertyId::BackgroundColor);
      auto disappearsIntoBackground =
        BlendSourceOver(parentBackgroundColor, elementBackgroundColor) == parentBackgroundColor;
      if (disappearsIntoBackground)
      {
        return parentStyle->VisitedDependentColorWithColorFilter(css::PropertyId::CaretColor);
      }
    }
    return elementStyle.VisitedDependentColorWithColorFilter(css::PropertyId::CaretColor);
  }

  void CaretBase::PaintCaret(const dom::Node &node, GraphicsContext &context, const LayoutPoint &paintOffset,
                             CaretAnimator *caretAnimator) const noexcept
  {
    auto caretPresentationProperties =
      caretAnimator ? caretAnimator->GetPresentationProperties() : CaretAnimator::PresentationProperties();
    if (_caretVisibility == CaretVisibility::Hidden
        || caretPresentationProperties.Blink == CaretAnimator::PresentationProperties::BlinkState::Off)
    {
      return;
    }

    auto caret = LocalCaretRectWithoutUpdate();
    if (auto *renderer = RendererForCaretPainting(&node))
    {
      renderer->FlipForWritingMode(caret);
    }
    caret.MoveBy(paintOffset);
    if (caret.IsEmpty())
    {
      return;
    }

    Color caretColor = Color::black;
    auto *element = DynamicDowncast<dom::Element>(node);
    if (!element)
    {
      element = node.ParentElement();
    }
    if (element && element->Renderer())
    {
      caretColor = CaretBase::ComputeCaretColor(element->Renderer()->Style(), &node);
    }

    auto pixelSnappedCaretRect = SnapRectToDevicePixels(caret, node.NodeDocument().DeviceScaleFactor());
    if (caretAnimator)
    {
      caretAnimator->Paint(context, pixelSnappedCaretRect, caretColor, paintOffset);
    }
    else
    {
      context.FillRect(pixelSnappedCaretRect, caretColor);
    }
  }

  void FrameSelection::SetCaretBlinkingSuspended(bool suspended) noexcept
  {
    GetCaretAnimator().SetBlinkingSuspended(suspended);
  }

  bool FrameSelection::IsCaretBlinkingSuspended() const noexcept
  {
    return GetCaretAnimator().IsBlinkingSuspended();
  }

  void FrameSelection::CaretAnimationDidUpdate(CaretAnimator &) noexcept
  {
    InvalidateCaretRect();
  }

  dom::Document *FrameSelection::GetDocument() noexcept
  {
    return _document;
  }

  dom::Node *FrameSelection::CaretNode() noexcept
  {
    return Selection().VisibleStart().DeepEquivalent().DeprecatedNode();
  }

  void FrameSelection::AddCaretVisibilitySuppressionReason(CaretVisibilitySuppressionReason reason,
                                                           ShouldUpdateAppearance doAppearanceUpdate) noexcept
  {
    _caretVisibilitySuppressionReasons.Add(reason);
    UpdateCaretVisibility(doAppearanceUpdate);
  }

  void
    FrameSelection::RemoveCaretVisibilitySuppressionReason(CaretVisibilitySuppressionReason reason,
                                                           ShouldUpdateAppearance doAppearanceUpdate) noexcept
  {
    _caretVisibilitySuppressionReasons.Remove(reason);
    UpdateCaretVisibility(doAppearanceUpdate);
  }

  void FrameSelection::UpdateCaretVisibility(ShouldUpdateAppearance doAppearanceUpdate) noexcept
  {
    auto visibility =
      _caretVisibilitySuppressionReasons.IsEmpty() ? CaretVisibility::Visible : CaretVisibility::Hidden;

    if (GetCaretVisibility() == visibility)
    {
      return;
    }

    GetCaretAnimator().SetVisible(false);

    CaretBase::SetCaretVisibility(visibility);

    if (doAppearanceUpdate == ShouldUpdateAppearance::Yes)
    {
      _pendingSelectionUpdate = true;
    }
  }

  // FIXME: Focusing the anchor a caret is in when browsing with a caret (enclosingAnchorElement), and leaving
  // out frame elements, once there are frames.
  void FrameSelection::SetFocusedElementIfNeeded(EnumSet<SetSelectionOption> options) noexcept
  {
    if (IsNone() || !IsFocused())
    {
      return;
    }

    RefPtr<dom::Document> document = ShareRefPtr(_document);
    if (RefPtr<dom::Element> target = ShareRefPtr(_selection.RootEditableElement()))
    {
      // Walk up the DOM tree to search for an element to focus.
      if (auto *focusable = html::FocusAlgorithms::ClickFocusableInclusiveAncestor(*target))
      {
        dom::FocusOptions focusOptions;
        if (options.Contains(SetSelectionOption::ForBindings))
        {
          focusOptions.Trigger = dom::FocusTrigger::Bindings;
        }
        html::FocusAlgorithms::FocusingSteps(*focusable, nullptr, focusOptions);
        return;
      }
      html::FocusAlgorithms::FocusingSteps(*document);
    }
  }

  EditingStyle *FrameSelection::TypingStyle() const noexcept
  {
    return _typingStyle.get();
  }

  RefPtr<css::MutableStyleProperties> FrameSelection::CopyTypingStyle() const noexcept
  {
    if (!_typingStyle || !_typingStyle->Style())
    {
      return nullptr;
    }
    return _typingStyle->Style()->MutableCopy();
  }

  void FrameSelection::SetTypingStyle(RefPtr<EditingStyle> &&style) noexcept
  {
    _typingStyle = krys::move(style);
  }

  void FrameSelection::ClearTypingStyle() noexcept
  {
    _typingStyle = nullptr;
  }

  RefPtr<dom::Range> FrameSelection::AssociatedLiveRange() noexcept
  {
    if (!_associatedLiveRange)
    {
      if (auto range = _selection.Range(); ContainsEndpoints(_document, range))
      {
        _associatedLiveRange = CreateRef<dom::Range>(range->Start, range->End);
        _associatedLiveRange->DidAssociateWithSelection();
      }
    }
    return _associatedLiveRange;
  }

  void FrameSelection::DisassociateLiveRange() noexcept
  {
    if (auto previouslyAssociatedLiveRange = std::exchange(_associatedLiveRange, nullptr))
    {
      previouslyAssociatedLiveRange->DidDisassociateFromSelection();
    }
  }

  void FrameSelection::AssociateLiveRange(dom::Range &liveRange) noexcept
  {
    DisassociateLiveRange();
    _associatedLiveRange = ShareRefPtr(&liveRange);
    liveRange.DidAssociateWithSelection();
    UpdateFromAssociatedLiveRange();
  }

  void FrameSelection::UpdateFromAssociatedLiveRange() noexcept
  {
    krys_debug_assert(_associatedLiveRange);
    if (!ContainsEndpoints(_document, *_associatedLiveRange))
    {
      DisassociateLiveRange();
    }
    else
    {
      // Don't use VisibleSelection's constructor that takes a SimpleRange, because it uses
      // MakeDeprecatedLegacyPosition instead of MakeContainerOffsetPosition.
      auto start = dom::MakeContainerOffsetPosition(ShareRefPtr(_associatedLiveRange->StartContainer()),
                                                    static_cast<uint32>(_associatedLiveRange->StartOffset()));
      auto end = dom::MakeContainerOffsetPosition(ShareRefPtr(_associatedLiveRange->EndContainer()),
                                                  static_cast<uint32>(_associatedLiveRange->EndOffset()));
      auto options = DefaultSetSelectionOptions();
      options.Add(SetSelectionOption::MaintainLiveRange);
      SetSelection(VisibleSelection(start, end), options);
    }
  }

  // FIXME: Reusing the live range for selection updates (the shouldReuseLiveRangeForSelectionUpdate quirk),
  // with quirks.
  void FrameSelection::UpdateOrDisassociateLiveRange(bool shouldMaintainLiveRange) noexcept
  {
    if (!shouldMaintainLiveRange)
    {
      DisassociateLiveRange();
    }
  }

  bool FrameSelection::IsInDocumentTree() const noexcept
  {
    auto range = _selection.Range();
    return _document && range && _document->Contains(range->Start.Container.get())
           && _document->Contains(range->End.Container.get());
  }

  // FIXME: The live range of the DOM's Selection (associatedLiveRange), with the DOM's Selection.
  bool FrameSelection::IsConnectedToDocument() const noexcept
  {
    return IsInDocumentTree();
  }
}
