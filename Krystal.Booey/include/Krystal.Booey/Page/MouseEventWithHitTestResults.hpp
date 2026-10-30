#pragma once

#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  /// @brief A mouse event with what it hit.
  class MouseEventWithHitTestResults
  {
  private:
    PlatformMouseEvent _event;
    HitTestResult _hitTestResult;

  public:
    MouseEventWithHitTestResults(const PlatformMouseEvent &event, const HitTestResult &hitTestResult) noexcept
        : _event(event), _hitTestResult(hitTestResult)
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

    KRYS_NODISCARD LayoutPoint LocalPoint() const noexcept
    {
      return _hitTestResult.LocalPoint();
    }

    KRYS_NODISCARD bool IsOverLink() const noexcept
    {
      return _hitTestResult.IsOverLink();
    }

    KRYS_NODISCARD dom::Node *TargetNode() const noexcept
    {
      return _hitTestResult.TargetNode();
    }

    KRYS_NODISCARD Scrollbar *GetScrollbar() const noexcept
    {
      return _hitTestResult.GetScrollbar();
    }
  };
}
