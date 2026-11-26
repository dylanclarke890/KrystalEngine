/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Page/Scrolling/ScrollAnchoringController.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include "Krystal.Web/Html/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Web/Rendering/Layers/RenderLayerScrollableArea.hpp"
#include "Krystal.Web/Rendering/RenderBox.hpp"
#include "Krystal.Web/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Web/Rendering/RenderView.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <utility>

namespace krys::web
{
  namespace
  {
    KRYS_NODISCARD bool ElementIsScrollableArea(const Element &element, const ScrollableArea &scrollableArea)
    {
      auto *box = element.GetRenderBox();
      return box && box->Layer() && box->Layer()->GetScrollableArea() == &scrollableArea;
    }

    /// @brief The element whose box scrolls: the document element for the frame view.
    KRYS_NODISCARD Element *ElementForScrollableArea(ScrollableArea &scrollableArea)
    {
      if (auto *layerScrollableArea = DynamicDowncast<RenderLayerScrollableArea>(scrollableArea))
      {
        return layerScrollableArea->Layer().Renderer().GetElement();
      }
      if (auto *document = Downcast<web::FrameView>(scrollableArea).Frame().Document())
      {
        return dom::TreeQueries::DocumentElement(*document);
      }
      return nullptr;
    }

    /// @brief The scroller's rect in absolute coordinates: the layout viewport for the frame view.
    KRYS_NODISCARD RectI BoundingRectForScrollableArea(ScrollableArea &scrollableArea)
    {
      if (auto *layerScrollableArea = DynamicDowncast<RenderLayerScrollableArea>(scrollableArea))
      {
        return layerScrollableArea->Layer().Renderer().AbsoluteBoundingBoxRect();
      }
      return SnappedRectI(Downcast<web::FrameView>(scrollableArea).LayoutViewportRect());
    }

    /// @brief The element itself or its closest ancestor that's a box: what a priority candidate anchors to.
    KRYS_NODISCARD RefPtr<Element> AnchorElementForPriorityCandidate(Element *element)
    {
      while (element)
      {
        if (auto *renderer = element->Renderer())
        {
          if (!renderer->IsAnonymousBlock() && (!renderer->IsInline() || renderer->IsAtomicInlineLevelBox()))
          {
            return ShareRefPtr(element);
          }
        }
        element = element->ParentElement();
      }
      return nullptr;
    }

    KRYS_NODISCARD ScrollAnchoringController *ScrollAnchoringControllerForElement(const Element &element)
    {
      auto *renderer = DynamicDowncast<RenderLayerModelObject>(element.Renderer());
      if (renderer && renderer->HasLayer())
      {
        if (auto *scrollableArea = renderer->Layer()->GetScrollableArea())
        {
          return scrollableArea->GetScrollAnchoringController();
        }
      }
      return nullptr;
    }

    /// @brief Whether @p element, in the chain from @p priorityCandidateElement up to @p parentController's
    /// scroller, is a viable candidate, and isn't a scroller keeping an anchor of its own.
    KRYS_NODISCARD bool CanIncludeElementInPriorityCandidateChain(Element &element,
                                                                  const Element &priorityCandidateElement,
                                                                  ScrollAnchoringController &parentController)
    {
      auto candidateResult = parentController.ExamineAnchorCandidate(element);
      auto *elementsController = ScrollAnchoringControllerForElement(element);
      return !(
        candidateResult == CandidateExaminationResult::Exclude
        || (&element == &priorityCandidateElement && candidateResult == CandidateExaminationResult::Skip)
        || (elementsController && elementsController->AnchorElement()));
    }

    KRYS_NODISCARD bool AbsolutePositionedElementOutsideScroller(const RenderElement &renderer,
                                                                 ScrollableArea &scroller)
    {
      auto *layerScrollableArea = DynamicDowncast<RenderLayerScrollableArea>(scroller);
      auto *layerModelObject = DynamicDowncast<RenderLayerModelObject>(renderer);
      if (layerScrollableArea && layerModelObject && layerModelObject->HasLayer())
      {
        return !layerModelObject->Layer()->AncestorLayerIsInContainingBlockChain(
          layerScrollableArea->Layer());
      }
      return false;
    }

    KRYS_NODISCARD bool CanDescendIntoElement(const Element &element)
    {
      if (auto *scrollAnchoringController = ScrollAnchoringControllerForElement(element))
      {
        return !scrollAnchoringController->AnchorElement();
      }
      return false;
    }

    /// @brief Whether typing in @p element can move what's around it: a text control or editable content.
    KRYS_NODISCARD bool IsEditablePriorityCandidate(const Element &element)
    {
      return element.IsTextField() || element.HasEditableStyle();
    }
  }

  ScrollAnchoringController::ScrollAnchoringController(ScrollableArea &owningScrollableArea) noexcept
      : _owningScrollableArea(owningScrollableArea)
  {
  }

  web::FrameView &ScrollAnchoringController::FrameView() const noexcept
  {
    if (auto *layerScrollableArea = DynamicDowncast<RenderLayerScrollableArea>(_owningScrollableArea))
    {
      return layerScrollableArea->Layer().Renderer().View().FrameView();
    }
    return Downcast<web::FrameView>(_owningScrollableArea);
  }

  void ScrollAnchoringController::InvalidateAnchorElement() noexcept
  {
    if (_midUpdatingScrollPositionForAnchorElement)
    {
      return;
    }
    if (!_anchorElement)
    {
      if (auto *element = ElementForScrollableArea(_owningScrollableArea))
      {
        if (auto *renderer = element->Renderer())
        {
          auto *controller = RenderObject::SearchParentChainForScrollAnchoringController(*renderer);
          if (controller && controller != this && controller->IsInScrollAnchoringAncestorChain(*renderer))
          {
            controller->InvalidateAnchorElement();
          }
        }
      }
    }
    _anchorElement = nullptr;
    _lastOffsetForAnchorElement = {};
    _isQueuedForScrollPositionUpdate = false;
    FrameView().DequeueScrollableAreaForScrollAnchoringUpdate(_owningScrollableArea);
  }

  PointF
    ScrollAnchoringController::ComputeOffsetFromOwningScroller(const RenderObject &candidate) const noexcept
  {
    return PointF(candidate.AbsoluteBoundingBoxRect().Location()
                  - BoundingRectForScrollableArea(_owningScrollableArea).Location());
  }

  void ScrollAnchoringController::NotifyChildHadSuppressingStyleChange() noexcept
  {
    _shouldSuppressScrollPositionUpdate = true;
  }

  bool ScrollAnchoringController::IsInScrollAnchoringAncestorChain(const RenderObject &object) const noexcept
  {
    RefPtr<Element> element = ShareRefPtr(_anchorElement.get());
    while (element)
    {
      if (element->Renderer() == &object)
      {
        return true;
      }
      if (ElementIsScrollableArea(*element, _owningScrollableArea))
      {
        break;
      }
      element = ShareRefPtr(element->ParentElementInComposedTree());
    }
    return false;
  }

  bool ScrollAnchoringController::DidFindPriorityCandidate(Document &document) noexcept
  {
    auto *scrollerElement = ElementForScrollableArea(_owningScrollableArea);
    auto viablePriorityCandidateForElement = [&](Element *element) -> RefPtr<Element>
    {
      RefPtr<Element> candidateElement = AnchorElementForPriorityCandidate(element);
      if (!candidateElement || candidateElement.get() == scrollerElement)
      {
        return nullptr;
      }

      RefPtr<Element> ancestor = candidateElement;
      while (ancestor && ancestor.get() != scrollerElement)
      {
        if (!CanIncludeElementInPriorityCandidateChain(*ancestor, *candidateElement, *this))
        {
          return nullptr;
        }
        ancestor = ShareRefPtr(ancestor->ParentElement());
      }

      // The candidate must be inside the scroller.
      if (ancestor.get() != scrollerElement)
      {
        return nullptr;
      }
      return candidateElement;
    };

    // Find-in-page's active match isn't a candidate: there's no find-in-page.
    auto *focusedElement = document.FocusedElement();
    if (!focusedElement || !IsEditablePriorityCandidate(*focusedElement))
    {
      return false;
    }
    if (RefPtr<Element> priorityCandidate = viablePriorityCandidateForElement(focusedElement))
    {
      _anchorElement = CreateWeakPtr(priorityCandidate.get());
      _lastOffsetForAnchorElement = ComputeOffsetFromOwningScroller(*priorityCandidate->Renderer());
      return true;
    }
    return false;
  }

  CandidateExaminationResult ScrollAnchoringController::ExamineAnchorCandidate(Element &element) noexcept
  {
    auto *scrollerElement = ElementForScrollableArea(_owningScrollableArea);
    if (scrollerElement == &element)
    {
      return CandidateExaminationResult::Skip;
    }

    // The scrollport, less its scroll padding.
    auto containingRect = BoundingRectForScrollableArea(_owningScrollableArea);
    if (auto *box = scrollerElement ? scrollerElement->GetRenderBox() : nullptr)
    {
      LayoutRect paddedLayerBounds(containingRect);
      Contract(paddedLayerBounds, box->ScrollPaddingForViewportRect(paddedLayerBounds));
      containingRect = SnappedRectI(paddedLayerBounds);
    }

    auto *renderer = element.Renderer();
    if (!renderer)
    {
      return CandidateExaminationResult::Skip;
    }

    auto boxRect = renderer->AbsoluteBoundingBoxRect();
    bool intersects = containingRect.Intersects(boxRect);

    // The excluded subtrees.
    const auto &style = renderer->Style();
    if (style.OverflowAnchor() == OverflowAnchor::None || renderer->IsStickilyPositioned()
        || renderer->IsFixedPositioned() || renderer->IsPseudoElement() || renderer->IsAnonymousBlock()
        || (renderer->IsAbsolutelyPositioned()
            && AbsolutePositionedElementOutsideScroller(*renderer, _owningScrollableArea))
        || (!intersects && style.UsedContain().Contains(style::ContainValue::Paint)))
    {
      return CandidateExaminationResult::Exclude;
    }

    auto *document = FrameView().Frame().Document();
    if ((document && &element == document->BodyOrFrameset()) || Is<HTMLHtmlElement>(element)
        || (renderer->IsInline() && !renderer->IsAtomicInlineLevelBox()))
    {
      return CandidateExaminationResult::Skip;
    }
    if (!boxRect.Width() || !boxRect.Height())
    {
      return CandidateExaminationResult::Skip;
    }
    if (containingRect.Contains(boxRect))
    {
      return CandidateExaminationResult::Select;
    }
    auto *renderBox = DynamicDowncast<RenderBox>(*renderer);
    bool isScrollingNode = renderBox && renderBox->HasPotentiallyScrollableOverflow();
    if (intersects)
    {
      return !isScrollingNode || CanDescendIntoElement(element) ? CandidateExaminationResult::Descend
                                                                : CandidateExaminationResult::Select;
    }
    if (isScrollingNode)
    {
      return CandidateExaminationResult::Exclude;
    }
    return CandidateExaminationResult::Skip;
  }

  Element *ScrollAnchoringController::FindAnchorElementRecursive(Element *element) noexcept
  {
    if (!element)
    {
      return nullptr;
    }

    auto result = ExamineAnchorCandidate(*element);
    switch (result)
    {
      case CandidateExaminationResult::Select:  return element;
      case CandidateExaminationResult::Exclude: return nullptr;
      case CandidateExaminationResult::Skip:
      case CandidateExaminationResult::Descend:
        for (auto *child = element->FirstElementChild().get(); child;
             child = child->NextElementSibling().get())
        {
          if (auto *anchorElement = FindAnchorElementRecursive(child))
          {
            return anchorElement;
          }
        }
        break;
    }
    return result == CandidateExaminationResult::Skip ? nullptr : element;
  }

  void ScrollAnchoringController::ChooseAnchorElement(Document &document) noexcept
  {
    // `overflow-anchor: none` on the scroller (the root, for the view) opts it out, as in Chrome.
    auto *scrollerElement = ElementForScrollableArea(_owningScrollableArea);
    auto *scrollerRenderer = scrollerElement ? scrollerElement->Renderer() : nullptr;
    if (!scrollerRenderer || scrollerRenderer->Style().OverflowAnchor() == OverflowAnchor::None)
    {
      return;
    }
    if (DidFindPriorityCandidate(document))
    {
      return;
    }
    if (_anchorElement)
    {
      return;
    }

    auto *anchorElement = FindAnchorElementRecursive(scrollerElement);
    if (!anchorElement)
    {
      return;
    }
    _anchorElement = CreateWeakPtr(anchorElement);
    _lastOffsetForAnchorElement = ComputeOffsetFromOwningScroller(*anchorElement->Renderer());
  }

  void ScrollAnchoringController::UpdateAnchorElement() noexcept
  {
    if (_owningScrollableArea.GetScrollPosition().IsZero() || _isQueuedForScrollPositionUpdate
        || FrameView().LayoutContext().GetLayoutPhase() != FrameViewLayoutContext::LayoutPhase::OutsideLayout)
    {
      return;
    }

    RefPtr<web::Document> document = ShareRefPtr(FrameView().Frame().Document());
    if (!document)
    {
      return;
    }

    if (!_anchorElement)
    {
      ChooseAnchorElement(*document);
      if (!_anchorElement)
      {
        return;
      }
    }
    _isQueuedForScrollPositionUpdate = true;
    FrameView().QueueScrollableAreaForScrollAnchoringUpdate(_owningScrollableArea);
  }

  void ScrollAnchoringController::AdjustScrollPositionForAnchoring() noexcept
  {
    auto queued = std::exchange(_isQueuedForScrollPositionUpdate, false);
    auto suppressed = std::exchange(_shouldSuppressScrollPositionUpdate, false);
    if (!_anchorElement || !queued)
    {
      return;
    }
    auto *renderer = _anchorElement->Renderer();
    if (!renderer || suppressed)
    {
      InvalidateAnchorElement();
      UpdateAnchorElement();
      return;
    }
    SizeF adjustment = ComputeOffsetFromOwningScroller(*renderer) - _lastOffsetForAnchorElement;
    if (adjustment.IsZero())
    {
      return;
    }
    SetForScope midUpdatingScrollPositionForAnchorElement(_midUpdatingScrollPositionForAnchorElement, true);
    auto newScrollPosition =
      _owningScrollableArea.GetScrollPosition()
      + SizeI(static_cast<int32>(adjustment.Width()), static_cast<int32>(adjustment.Height()));
    auto oldScrollType = _owningScrollableArea.CurrentScrollType();
    _owningScrollableArea.SetCurrentScrollType(ScrollType::Programmatic);
    if (!_owningScrollableArea.RequestScrollToPosition(newScrollPosition,
                                                       ScrollPositionChangeOptions::CreateProgrammatic()))
    {
      (void)_owningScrollableArea.ScrollToPositionWithoutAnimation(newScrollPosition);
    }
    _owningScrollableArea.SetCurrentScrollType(oldScrollType);
  }
}
