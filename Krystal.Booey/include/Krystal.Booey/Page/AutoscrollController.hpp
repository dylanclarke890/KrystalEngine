#pragma once

#include "Krystal.Booey/Graphics/IntPoint.hpp"
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
  class RenderBox;
  class RenderObject;

  /// @brief Scrolls the box a mouse drag that's selecting is in, while the mouse is held near or past its
  /// edge, and the box a drag and drop is over, while it's held near its edge.
  // FIXME: Pan scrolling (the middle button), with it.
  class AutoscrollController
  {
    KRYS_NON_COPY_MOVABLE(AutoscrollController);

    enum class AutoscrollType : uint8
    {
      None,
      Selection,
      DragAndDrop,
    };

  private:
    Timer _autoscrollTimer;
    WeakPtr<RenderBox> _autoscrollRenderer;
    AutoscrollType _autoscrollType {AutoscrollType::None};
    IntPoint _dragAndDropAutoscrollReferencePosition;
    MonotonicTime::duration _dragAndDropAutoscrollStartTime {};

  public:
    AutoscrollController() noexcept;
    ~AutoscrollController() noexcept;

    KRYS_NODISCARD RenderBox *AutoscrollRenderer() const noexcept;
    KRYS_NODISCARD bool AutoscrollInProgress() const noexcept;
    KRYS_NODISCARD bool PanScrollInProgress() const noexcept;

    void StartAutoscrollForSelection(RenderObject *renderer) noexcept;
    /// @brief A drag and drop moved over @p targetNode at @p eventPosition (in the window): scrolls the box
    /// it's near the edge of, after a moment.
    void UpdateDragAndDrop(dom::Node *targetNode, const IntPoint &eventPosition,
                           MonotonicTime::duration eventTime) noexcept;
    void StopAutoscrollTimer(bool rendererIsBeingDestroyed = false) noexcept;

  private:
    void AutoscrollTimerFired() noexcept;
    void StartAutoscrollTimer() noexcept;
  };
}
