#include "Krystal.Booey/Platform/SimpleCaretAnimator.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo
{
  SimpleCaretAnimator::SimpleCaretAnimator(CaretAnimationClient &client) noexcept : CaretAnimator(client)
  {
  }

  void SimpleCaretAnimator::UpdateAnimationProperties() noexcept
  {
    auto currentTime = Now();
    auto caretBlinkInterval = RenderTheme::Singleton().CaretBlinkInterval();

    SetBlinkingSuspended(!caretBlinkInterval);

    if (IsBlinkingSuspended())
    {
      // Ensure the caret is always visible when blinking is suspended.
      if (_presentationProperties.Blink == PresentationProperties::BlinkState::On)
      {
        _blinkTimer.StartOneShot(caretBlinkInterval.value_or(MonotonicTime::duration {}));
      }
      return;
    }

    krys_debug_assert(caretBlinkInterval.has_value());

    if (currentTime - _lastTimeCaretPaintWasToggled >= *caretBlinkInterval)
    {
      SetBlinkState(!_presentationProperties.Blink);
      _lastTimeCaretPaintWasToggled = currentTime;

      _blinkTimer.StartOneShot(*caretBlinkInterval);
    }
  }

  void SimpleCaretAnimator::Start() noexcept
  {
    _lastTimeCaretPaintWasToggled = Now();
    DidStart(_lastTimeCaretPaintWasToggled, RenderTheme::Singleton().CaretBlinkInterval());
  }
}
