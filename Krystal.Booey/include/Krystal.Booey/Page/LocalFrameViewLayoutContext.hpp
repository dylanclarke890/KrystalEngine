#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class LocalFrame;
  class LocalFrameView;
  class RenderView;

  /// @brief Schedules and runs a view's layout: the style updates layout needs first, then the layout of the
  /// render tree.
  /// @note FIXME: There's no render tree to lay out: layout updates style (re-evaluating the media queries
  /// that depend on the viewport) and stops where the render tree's layout would run. There's no layout timer
  /// either: a scheduled layout is pending until the host runs it (`Layout()` or `Document::UpdateLayout()`).
  class LocalFrameViewLayoutContext final
  {
    KRYS_NON_COPY_MOVABLE(LocalFrameViewLayoutContext);

  public:
    enum class LayoutPhase : uint8
    {
      OutsideLayout,
      InPreLayout,
      InRenderTreeLayout,
      InViewSizeAdjust,
      InPostLayout
    };

  private:
    enum class LayoutNestedState : uint8
    {
      NotInLayout,
      NotNested,
      Nested
    };

    LocalFrameView &_frameView;
    LayoutPhase _layoutPhase {LayoutPhase::OutsideLayout};
    LayoutNestedState _layoutNestedState {LayoutNestedState::NotInLayout};
    // Stands in for the layout timer: whether a layout is scheduled.
    bool _layoutPending {false};
    bool _layoutSchedulingIsEnabled {true};
    bool _firstLayout {true};
    bool _setNeedsLayoutWasDeferred {false};
    uint32 _disableSetNeedsLayoutCount {0};

  public:
    explicit LocalFrameViewLayoutContext(LocalFrameView &frameView) noexcept;
    ~LocalFrameViewLayoutContext() noexcept;

    void Layout() noexcept;
    KRYS_NODISCARD bool NeedsLayout() const noexcept;

    // We rely on the side-effects of layout, like compositing updates, to update state in various subsystems
    // whose dependencies are poorly defined. This call triggers such updates.
    void SetNeedsLayoutAfterViewConfigurationChange() noexcept;

    void ScheduleLayout() noexcept;
    void UnscheduleLayout() noexcept;

    void DisableSetNeedsLayout() noexcept;
    void EnableSetNeedsLayout() noexcept;

    KRYS_NODISCARD LayoutPhase GetLayoutPhase() const noexcept
    {
      return _layoutPhase;
    }

    KRYS_NODISCARD bool IsLayoutNested() const noexcept
    {
      return _layoutNestedState == LayoutNestedState::Nested;
    }

    KRYS_NODISCARD bool IsLayoutPending() const noexcept
    {
      return _layoutPending;
    }

    KRYS_NODISCARD bool IsInLayout() const noexcept
    {
      return _layoutPhase != LayoutPhase::OutsideLayout;
    }

    KRYS_NODISCARD bool IsInRenderTreeLayout() const noexcept
    {
      return _layoutPhase == LayoutPhase::InRenderTreeLayout;
    }

    void ResetFirstLayoutFlag() noexcept
    {
      _firstLayout = true;
    }

    KRYS_NODISCARD bool DidFirstLayout() const noexcept
    {
      return !_firstLayout;
    }

    KRYS_NODISCARD LocalFrameView &View() const noexcept
    {
      return _frameView;
    }

    KRYS_NODISCARD LocalFrame &Frame() const noexcept;
    KRYS_NODISCARD dom::Document *Document() const noexcept;
    KRYS_NODISCARD RenderView *GetRenderView() const noexcept;

  private:
    KRYS_NODISCARD bool NeedsLayoutInternal() const noexcept;

    void PerformLayout() noexcept;
    KRYS_NODISCARD bool CanPerformLayout() const noexcept;

    KRYS_NODISCARD bool IsLayoutSchedulingEnabled() const noexcept
    {
      return _layoutSchedulingIsEnabled;
    }

    void UpdateStyleForLayout() noexcept;
  };
}
