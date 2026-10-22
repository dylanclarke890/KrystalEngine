#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/RenderedPosition.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

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
      : _document(document), _focused(document && document->IsFocusedAndActive())
  {
    if (ShouldAlwaysUseDirectionalSelection(_document))
    {
      _selection.SetDirectionality(Directionality::Strong);
    }

    if (IsFocusedAndActive())
    {
      SetSelectionFromNone();
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
    const VisibleSelection &newSelectionPossiblyWithoutDirection, EnumSet<SetSelectionOption> /*options*/,
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

    _granularity = granularity;
    SetNodeFlags(_selection, false);
    _selection = newSelection;
    SetNodeFlags(_selection, true);

    return willMutateSelection;
  }

  void FrameSelection::SetSelection(const VisibleSelection &selection, EnumSet<SetSelectionOption> options,
                                    CursorAlignOnScroll align, TextGranularity granularity) noexcept
  {
    Ref<dom::Document> document = ShareRef(*_document);
    if (!SetSelectionWithoutUpdatingAppearance(selection, options, align, granularity))
    {
      return;
    }

    _pendingSelectionUpdate = true;

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

    UpdateAndRevealSelection();
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

  // FIXME: Revealing the selection (revealSelection) and accessibility notifications, with the keyboard.
  void FrameSelection::UpdateAndRevealSelection() noexcept
  {
    if (!_pendingSelectionUpdate)
    {
      return;
    }

    _pendingSelectionUpdate = false;

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

    if (auto *view = _document->GetRenderView())
    {
      view->Selection().Clear();
    }

    auto options = DefaultSetSelectionOptions();
    options.Add(SetSelectionOption::DoNotNotifyEditorClients);
    SetSelectionWithoutUpdatingAppearance(VisibleSelection(), options, CursorAlignOnScroll::IfNeeded,
                                          TextGranularity::CharacterGranularity);
    _originalBase = {};
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

  // FIXME: Select elements and text fields (selectAll on a focused select or text field), with form controls.
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
    Ref<dom::Document> document = ShareRef(*_document);
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

  void FrameSelection::UpdateAppearance() noexcept
  {
    // Construct a new VisibleSolution, since _selection is not necessarily valid, and the following steps
    // assume a valid selection.
    VisibleSelection oldSelection = Selection();
    VisibleSelection selection(oldSelection.VisibleStart(), oldSelection.VisibleEnd());

    auto *document = _document;
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
    UpdateAndRevealSelection();
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
