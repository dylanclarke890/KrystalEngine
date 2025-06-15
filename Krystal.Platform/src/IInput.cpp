#include "Krystal.Platform/IInput.hpp"

#include "Krystal.Platform/Keyboard.hpp"
#include "Krystal.Platform/Keys.hpp"
#include "Krystal.Platform/Mouse.hpp"
#include "Krystal.Platform/MouseButtons.hpp"

namespace Krys::Platform
{
  const Keyboard &IInput::GetKeyboard() const noexcept
  {
    return _keyboard;
  }

  const Mouse &IInput::GetMouse() const noexcept
  {
    return _mouse;
  }

  void IInput::OnKeyboardEvent(const Key key, const KeyState state) noexcept
  {
    switch (state)
    {
      case KeyState::Pressed:  _keyboard._pressed.emplace(key); break;
      case KeyState::Held:     _keyboard._held.emplace(key); break;
      case KeyState::Released: _keyboard._released.emplace(key); break;
    }
  }

  void IInput::OnMouseButtonEvent(const MouseButton button, const MouseButtonState state) noexcept
  {
    switch (state)
    {
      case MouseButtonState::Pressed:  _mouse._pressed |= button; break;
      case MouseButtonState::Released: _mouse._released |= button; break;
    }
  }

  void IInput::OnMouseMoveEvent(const float deltaX, const float deltaY, const float clientX,
                        const float clientY) noexcept
  {
    _mouse._deltaX += deltaX;
    _mouse._deltaY += deltaY;
    _mouse._clientX = clientX;
    _mouse._clientY = clientY;
  }

  void IInput::OnScrollWheelEvent(const float delta) noexcept
  {
    _mouse._scrollDelta += delta;
  }

  void IInput::ClearState() noexcept
  {
    _mouse._deltaX = 0;
    _mouse._deltaY = 0;
    _mouse._scrollDelta = 0;
    _mouse._held |= _mouse._pressed;
    _mouse._held &= ~_mouse._released;
    _mouse._pressed = MouseButton::None;
    _mouse._released = MouseButton::None;

    _keyboard._pressed.clear();
    _keyboard._held.clear();
    _keyboard._released.clear();
  }
}