#pragma once

#include "Krystal.Booey/Platform/CaretAnimator.hpp"

namespace krys::boo
{
  /// @brief A caret that blinks on and off at the theme's interval.
  class SimpleCaretAnimator final : public CaretAnimator
  {
  private:
    MonotonicTime::duration _lastTimeCaretPaintWasToggled {};

  public:
    explicit SimpleCaretAnimator(CaretAnimationClient &client) noexcept;

  private:
    void UpdateAnimationProperties() noexcept final;
    void Start() noexcept final;

    void SetVisible(bool visible) noexcept final
    {
      SetBlinkState(visible ? PresentationProperties::BlinkState::On
                            : PresentationProperties::BlinkState::Off);
    }

    void SetBlinkState(PresentationProperties::BlinkState blinkState) noexcept
    {
      if (_presentationProperties.Blink == blinkState)
      {
        return;
      }
      _presentationProperties.Blink = blinkState;
      _client.CaretAnimationDidUpdate(*this);
    }
  };
}
