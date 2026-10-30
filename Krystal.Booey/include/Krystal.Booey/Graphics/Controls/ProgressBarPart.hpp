#pragma once

#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"

namespace krys::boo
{
  /// @brief A progress bar: how far along it is (negative when indeterminate), and when its indeterminate
  /// animation started.
  class ProgressBarPart : public ControlPart
  {
  private:
    double _position;
    MonotonicTime::duration _animationStartTime;

  public:
    KRYS_NODISCARD static Ref<ProgressBarPart> Create() noexcept;
    KRYS_NODISCARD static Ref<ProgressBarPart> Create(double position,
                                                      MonotonicTime::duration animationStartTime) noexcept;

    KRYS_NODISCARD double Position() const noexcept
    {
      return _position;
    }

    void SetPosition(double position) noexcept
    {
      _position = position;
    }

    KRYS_NODISCARD MonotonicTime::duration AnimationStartTime() const noexcept
    {
      return _animationStartTime;
    }

    void SetAnimationStartTime(MonotonicTime::duration animationStartTime) noexcept
    {
      _animationStartTime = animationStartTime;
    }

  private:
    ProgressBarPart(double position, MonotonicTime::duration animationStartTime) noexcept;

    KRYS_NODISCARD UniquePtr<PlatformControl> CreatePlatformControl() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_CONTROL_PART(ProgressBar)
