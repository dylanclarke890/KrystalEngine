#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Web/Platform/Events/PlatformMouseEvent.hpp"
#include "Krystal.Web/Rendering/HitTesting/HitTestResult.hpp"

namespace krys::web
{
  /// @brief A mouse event together with the result of hit testing its point, as the event handler passes
  /// them around.
  class MouseEventWithHitTestResults
  {
  private:
    PlatformMouseEvent _event;
    HitTestResult _hitTestResult;

  public:
    MouseEventWithHitTestResults(PlatformMouseEvent event, const HitTestResult &hitTestResult) noexcept
        : _event(krys::move(event)), _hitTestResult(hitTestResult)
    {
    }

    KRYS_NODISCARD const PlatformMouseEvent &Event() const noexcept
    {
      return _event;
    }

    KRYS_NODISCARD const HitTestResult &GetHitTestResult() const noexcept
    {
      return _hitTestResult;
    }

    /// @brief The point in the coordinates of the hit node's renderer.
    KRYS_NODISCARD LayoutPoint LocalPoint() const noexcept
    {
      return _hitTestResult.LocalPoint();
    }

    /// @brief Whether the point is over a link.
    KRYS_NODISCARD bool IsOverLink() const noexcept
    {
      return _hitTestResult.IsOverLink();
    }

    /// @brief The innermost node hit.
    KRYS_NODISCARD Node *TargetNode() const noexcept
    {
      return _hitTestResult.TargetNode();
    }

    /// @brief The scrollbar the point is over, if any.
    KRYS_NODISCARD Scrollbar *GetScrollbar() const noexcept
    {
      return _hitTestResult.GetScrollbar();
    }
  };
}
