#include "Krystal.Booey/Page/AutoscrollController.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"

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
    return false;
  }

  // FIXME: List boxes (RenderListBox), with form controls.
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

  // FIXME: Subframes (mouseDownWasInSubframe), once there are frames.
  void AutoscrollController::StopAutoscrollTimer(bool rendererIsBeingDestroyed) noexcept
  {
    auto *scrollable = _autoscrollRenderer.get();
    _autoscrollTimer.Stop();
    _autoscrollRenderer = nullptr;

    if (!scrollable)
    {
      return;
    }

    if (!rendererIsBeingDestroyed)
    {
      scrollable->StopAutoscroll();
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
