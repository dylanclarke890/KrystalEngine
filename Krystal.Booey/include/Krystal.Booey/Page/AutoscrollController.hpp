#pragma once

#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderBox;
  class RenderObject;

  /// @brief Scrolls the box a mouse drag that's selecting is in, while the mouse is held near or past its
  /// edge.
  // FIXME: Pan scrolling (the middle button) and autoscrolling while dragging and dropping, with them.
  class AutoscrollController
  {
    KRYS_NON_COPY_MOVABLE(AutoscrollController);

    enum class AutoscrollType : uint8
    {
      None,
      Selection,
    };

  private:
    Timer _autoscrollTimer;
    WeakPtr<RenderBox> _autoscrollRenderer;
    AutoscrollType _autoscrollType {AutoscrollType::None};

  public:
    AutoscrollController() noexcept;
    ~AutoscrollController() noexcept;

    KRYS_NODISCARD RenderBox *AutoscrollRenderer() const noexcept;
    KRYS_NODISCARD bool AutoscrollInProgress() const noexcept;
    KRYS_NODISCARD bool PanScrollInProgress() const noexcept;

    void StartAutoscrollForSelection(RenderObject *renderer) noexcept;
    void StopAutoscrollTimer(bool rendererIsBeingDestroyed = false) noexcept;

  private:
    void AutoscrollTimerFired() noexcept;
    void StartAutoscrollTimer() noexcept;
  };
}
