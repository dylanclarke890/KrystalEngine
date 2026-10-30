#include "Krystal.Booey/Graphics/Controls/ProgressBarPart.hpp"

namespace krys::boo
{
  Ref<ProgressBarPart> ProgressBarPart::Create() noexcept
  {
    return AdoptRef(*new ProgressBarPart(0, {}));
  }

  Ref<ProgressBarPart> ProgressBarPart::Create(double position,
                                               MonotonicTime::duration animationStartTime) noexcept
  {
    return AdoptRef(*new ProgressBarPart(position, animationStartTime));
  }

  ProgressBarPart::ProgressBarPart(double position, MonotonicTime::duration animationStartTime) noexcept
      : ControlPart(StyleAppearance::ProgressBar), _position(position),
        _animationStartTime(animationStartTime)
  {
  }

  UniquePtr<PlatformControl> ProgressBarPart::CreatePlatformControl() noexcept
  {
    return GetControlFactory().CreatePlatformProgressBar(*this);
  }
}
