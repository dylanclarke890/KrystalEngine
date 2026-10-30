#pragma once

#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"

namespace krys::boo::html
{
  class HTMLElement;
  class HTMLProgressElement;
}

namespace krys::boo
{
  /// @brief The renderer of a `<progress>` with a native appearance (or its inner element without one): a
  /// block the theme draws as a bar, animated while it's indeterminate.
  class RenderProgress final : public RenderBlockFlow
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderProgress);

  private:
    double _position;
    MonotonicTime::duration _animationStartTime {};
    bool _animating {false};
    Timer _animationTimer;

  public:
    RenderProgress(html::HTMLElement &, RenderStyle &&) noexcept;
    ~RenderProgress() noexcept override;

    KRYS_NODISCARD double Position() const noexcept
    {
      return _position;
    }

    KRYS_NODISCARD double AnimationProgress() const noexcept;

    KRYS_NODISCARD MonotonicTime::duration AnimationStartTime() const noexcept
    {
      return _animationStartTime;
    }

    KRYS_NODISCARD bool IsDeterminate() const noexcept;
    void UpdateFromElement() noexcept override;

    KRYS_NODISCARD html::HTMLProgressElement *ProgressElement() const noexcept;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderProgress";
    }

    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;
    void WillBeDestroyed() noexcept final;

    void AnimationTimerFired() noexcept;
    void UpdateAnimationState() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderProgress)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderProgress();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
