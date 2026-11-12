/*
 * Copyright (C) 2006-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
 * Copyright (C) 2006 Alexey Proskuryakov (ap@webkit.org)
 * Copyright (C) 2012 Digia Plc. and/or its subsidiary(-ies)
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

#include "Krystal.Booey/Page/AutoscrollController.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Events/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Rendering/FormControls/RenderListBox.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo
{
  namespace
  {
    constexpr MonotonicTime::duration AutoscrollDelay = std::chrono::milliseconds(200);
    constexpr MonotonicTime::duration AutoscrollInterval = std::chrono::milliseconds(50);
  }

  AutoscrollController::AutoscrollController() noexcept
      : _autoscrollTimer(*this, &AutoscrollController::AutoscrollTimerFired)
  {
  }

  AutoscrollController::~AutoscrollController() noexcept = default;

  RenderBox *AutoscrollController::AutoscrollRenderer() const noexcept
  {
    return _autoscrollRenderer.get();
  }

  bool AutoscrollController::AutoscrollInProgress() const noexcept
  {
    return _autoscrollType == AutoscrollType::Selection;
  }

  bool AutoscrollController::PanScrollInProgress() const noexcept
  {
    return _autoscrollType == AutoscrollType::Pan || _autoscrollType == AutoscrollType::PanCanStop;
  }

  void AutoscrollController::StartPanScrolling(RenderBox &scrollable,
                                               const IntPoint &lastKnownMousePosition) noexcept
  {
    // We don't want to trigger the autoscroll or the panScroll if it's already active
    if (_autoscrollTimer.IsActive())
    {
      return;
    }

    _autoscrollType = AutoscrollType::Pan;
    _autoscrollRenderer = CreateWeakPtr(&scrollable);
    _panScrollStartPosition = lastKnownMousePosition;
    _panScrollMousePosition = lastKnownMousePosition;

    scrollable.View().FrameView().AddPanScrollIcon(lastKnownMousePosition);
    StartAutoscrollTimer();
  }

  void AutoscrollController::HandleMouseReleaseEvent(const PlatformMouseEvent &mouseEvent) noexcept
  {
    if (_autoscrollType == AutoscrollType::Pan)
    {
      if (mouseEvent.Button() == MouseButton::Middle)
      {
        _autoscrollType = AutoscrollType::PanCanStop;
      }
    }
    else if (_autoscrollType == AutoscrollType::PanCanStop)
    {
      StopAutoscrollTimer();
    }
  }

  void AutoscrollController::UpdatePanScrollState(LocalFrame &frame,
                                                  const IntPoint &lastKnownMousePosition) noexcept
  {
    // Over the icon at the press there's no scrolling, so the cursor doesn't point anywhere.
    auto radius = LocalFrameView::NoPanScrollRadius;
    bool east = _panScrollStartPosition.X() < lastKnownMousePosition.X() - radius;
    bool west = _panScrollStartPosition.X() > lastKnownMousePosition.X() + radius;
    bool north = _panScrollStartPosition.Y() > lastKnownMousePosition.Y() + radius;
    bool south = _panScrollStartPosition.Y() < lastKnownMousePosition.Y() - radius;

    if (_autoscrollType == AutoscrollType::Pan && (east || west || north || south))
    {
      _autoscrollType = AutoscrollType::PanCanStop;
    }

    auto type = Cursor::Type::MiddlePanning;
    if (north)
    {
      type = east   ? Cursor::Type::NorthEastPanning
             : west ? Cursor::Type::NorthWestPanning
                    : Cursor::Type::NorthPanning;
    }
    else if (south)
    {
      type = east   ? Cursor::Type::SouthEastPanning
             : west ? Cursor::Type::SouthWestPanning
                    : Cursor::Type::SouthPanning;
    }
    else if (east)
    {
      type = Cursor::Type::EastPanning;
    }
    else if (west)
    {
      type = Cursor::Type::WestPanning;
    }
    frame.GetEventHandler().SetCurrentMouseCursor(Cursor(type));
  }

  void AutoscrollController::StartAutoscrollForSelection(RenderObject *renderer) noexcept
  {
    // We don't want to trigger the autoscroll or the panScroll if it's already active
    if (_autoscrollTimer.IsActive())
    {
      return;
    }
    RenderBox *scrollable = RenderBox::FindAutoscrollable(renderer);
    if (!scrollable)
    {
      scrollable = DynamicDowncast<RenderListBox>(renderer);
    }
    if (!scrollable)
    {
      return;
    }
    _autoscrollType = AutoscrollType::Selection;
    _autoscrollRenderer = CreateWeakPtr(scrollable);
    StartAutoscrollTimer();
  }

  void AutoscrollController::UpdateDragAndDrop(dom::Node *dropTargetNode, const IntPoint &eventPosition,
                                               MonotonicTime::duration eventTime) noexcept
  {
    IntSize offset;
    auto findDragAndDropScroller = [&]() -> RenderBox *
    {
      if (!dropTargetNode)
      {
        return nullptr;
      }

      auto *scrollable = RenderBox::FindAutoscrollable(dropTargetNode->Renderer());
      if (!scrollable)
      {
        return nullptr;
      }

      if (!scrollable->GetDocument().GetSettings().AutoscrollForDragAndDropEnabled)
      {
        return nullptr;
      }

      offset = scrollable->CalculateAutoscrollDirection(eventPosition);
      if (offset.IsZero())
      {
        return nullptr;
      }

      return scrollable;
    };

    auto *scrollable = findDragAndDropScroller();
    if (!scrollable)
    {
      StopAutoscrollTimer();
      return;
    }

    _dragAndDropAutoscrollReferencePosition = eventPosition + offset;

    if (_autoscrollType == AutoscrollType::None)
    {
      _autoscrollType = AutoscrollType::DragAndDrop;
      _autoscrollRenderer = CreateWeakPtr(scrollable);
      _dragAndDropAutoscrollStartTime = eventTime;
      StartAutoscrollTimer();
    }
    else if (_autoscrollRenderer.get() != scrollable)
    {
      _dragAndDropAutoscrollStartTime = eventTime;
      _autoscrollRenderer = CreateWeakPtr(scrollable);
    }
  }

  void AutoscrollController::StopAutoscrollTimer(bool rendererIsBeingDestroyed) noexcept
  {
    auto *scrollable = _autoscrollRenderer.get();
    _autoscrollTimer.Stop();
    _autoscrollRenderer = nullptr;

    if (!scrollable)
    {
      _autoscrollType = AutoscrollType::None;
      return;
    }

    if (!rendererIsBeingDestroyed)
    {
      scrollable->StopAutoscroll();

      // The scrollers it moved didn't snap as it went: they snap now it's ended, the innermost first.
      if (auto *layer = scrollable->EnclosingLayer())
      {
        for (auto *scrollableLayer = layer->EnclosingScrollableLayer(IncludeSelf, CrossFrameBoundaries::No);
             scrollableLayer && !scrollableLayer->IsRenderViewLayer();
             scrollableLayer =
               scrollableLayer->EnclosingScrollableLayer(ExcludeSelf, CrossFrameBoundaries::No))
        {
          if (auto *scrollableArea = scrollableLayer->GetScrollableArea())
          {
            scrollableArea->SnapAfterScrollEnds();
          }
        }
      }
      scrollable->View().FrameView().SnapAfterScrollEnds();
    }

    if (PanScrollInProgress())
    {
      scrollable->View().FrameView().RemovePanScrollIcon();
      if (auto *frame = scrollable->GetDocument().Frame())
      {
        frame->GetEventHandler().SetCurrentMouseCursor(Cursor(Cursor::Type::Pointer));
      }
    }

    _autoscrollType = AutoscrollType::None;
  }

  void AutoscrollController::AutoscrollTimerFired() noexcept
  {
    if (!_autoscrollRenderer)
    {
      StopAutoscrollTimer();
      return;
    }

    Ref<LocalFrame> frame = ShareRef(*_autoscrollRenderer->GetDocument().Frame());
    switch (_autoscrollType)
    {
      case AutoscrollType::DragAndDrop:
      {
        if (MonotonicTime::Now() - _dragAndDropAutoscrollStartTime > AutoscrollDelay)
        {
          _autoscrollRenderer->Autoscroll(_dragAndDropAutoscrollReferencePosition);
        }
        break;
      }
      case AutoscrollType::Pan:
      case AutoscrollType::PanCanStop:
      {
        if (auto position = frame->GetEventHandler().LastKnownMousePosition())
        {
          // With the mouse out of the window its position says nothing: panning goes on as it was.
          auto mousePosition = FlooredIntPoint(*position);
          if (mousePosition.X() >= 0 && mousePosition.Y() >= 0)
          {
            _panScrollMousePosition = mousePosition;
          }
        }
        UpdatePanScrollState(*frame, _panScrollMousePosition);
        _autoscrollRenderer->PanScroll(_panScrollStartPosition, _panScrollMousePosition);
        break;
      }
      case AutoscrollType::Selection:
      {
        if (!frame->GetEventHandler().ShouldUpdateAutoscroll())
        {
          StopAutoscrollTimer();
          return;
        }
        frame->GetEventHandler().UpdateSelectionForMouseDrag();
        if (auto *renderer = _autoscrollRenderer.get())
        {
          renderer->Autoscroll(frame->GetEventHandler().TargetPositionInWindowForSelectionAutoscroll());
        }
        break;
      }
      case AutoscrollType::None: break;
    }
  }

  void AutoscrollController::StartAutoscrollTimer() noexcept
  {
    _autoscrollTimer.StartRepeating(AutoscrollInterval);
  }
}
