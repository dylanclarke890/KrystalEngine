#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Node;
}

namespace krys::boo
{
  class LocalFrame;
  class PlatformMouseEvent;
  class RenderBox;
  class RenderObject;

  /// @brief Scrolls the box a mouse drag that's selecting is in, while the mouse is held near or past its
  /// edge, the box a drag and drop is over, while it's held near its edge, and the box a middle button press
  /// was in, towards wherever the mouse goes from the press (pan scrolling).
  class AutoscrollController
  {
    KRYS_NON_COPY_MOVABLE(AutoscrollController);

    enum class AutoscrollType : uint8
    {
      None,
      Selection,
      DragAndDrop,
      /// Panning with the mouse still on the icon: releasing the button that started it doesn't stop it.
      Pan,
      /// Panning, and any release of a mouse button stops it.
      PanCanStop,
    };

  private:
    Timer _autoscrollTimer;
    WeakPtr<RenderBox> _autoscrollRenderer;
    AutoscrollType _autoscrollType {AutoscrollType::None};
    IntPoint _dragAndDropAutoscrollReferencePosition;
    MonotonicTime::duration _dragAndDropAutoscrollStartTime {};
    IntPoint _panScrollStartPosition;
    IntPoint _panScrollMousePosition;

  public:
    AutoscrollController() noexcept;
    ~AutoscrollController() noexcept;

    KRYS_NODISCARD RenderBox *AutoscrollRenderer() const noexcept;
    KRYS_NODISCARD bool AutoscrollInProgress() const noexcept;
    KRYS_NODISCARD bool PanScrollInProgress() const noexcept;

    void StartAutoscrollForSelection(RenderObject *renderer) noexcept;
    /// @brief Starts panning @p scrollable from @p lastKnownMousePosition (in the window).
    void StartPanScrolling(RenderBox &scrollable, const IntPoint &lastKnownMousePosition) noexcept;
    /// @brief A mouse button was released: stops panning, unless it's the release of the press that started
    /// it and the mouse hasn't moved away.
    void HandleMouseReleaseEvent(const PlatformMouseEvent &mouseEvent) noexcept;
    /// @brief A drag and drop moved over @p targetNode at @p eventPosition (in the window): scrolls the box
    /// it's near the edge of, after a moment.
    void UpdateDragAndDrop(dom::Node *targetNode, const IntPoint &eventPosition,
                           MonotonicTime::duration eventTime) noexcept;
    void StopAutoscrollTimer(bool rendererIsBeingDestroyed = false) noexcept;

  private:
    void AutoscrollTimerFired() noexcept;
    void StartAutoscrollTimer() noexcept;
    void UpdatePanScrollState(LocalFrame &frame, const IntPoint &lastKnownMousePosition) noexcept;
  };
}
