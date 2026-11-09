#include "Krystal.Booey/Page/AutoscrollController.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Platform/Events/PlatformMouseEvent.hpp"
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

  // FIXME: List boxes (RenderListBox).
  void AutoscrollController::StartAutoscrollForSelection(RenderObject *renderer) noexcept
  {
    // We don't want to trigger the autoscroll or the panScroll if it's already active
    if (_autoscrollTimer.IsActive())
    {
      return;
    }
    auto *scrollable = RenderBox::FindAutoscrollable(renderer);
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
