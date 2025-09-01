#include "Krystal.Platform/Input.hpp"

namespace Krys::Platform
{
  Input::Input(Ptr<EventManager> events) noexcept : _mouse(), _keyboard(), _events(events)
  {
  }

  const Keyboard &Input::GetKeyboard() const noexcept
  {
    return _keyboard;
  }

  const Mouse &Input::GetMouse() const noexcept
  {
    return _mouse;
  }

  void Input::BeginFrame() noexcept
  {
    _mouse._deltaX = 0;
    _mouse._deltaY = 0;
    _mouse._scrollDelta = 0;
    _mouse._held |= _mouse._pressed;
    _mouse._held &= ~_mouse._released;
    _mouse._pressed = MouseButton::None;
    _mouse._released = MouseButton::None;

    for (const auto &key : _keyboard._pressed)
      _keyboard._held.emplace(key);

    for (const auto &key : _keyboard._released)
      _keyboard._held.erase(key);

    _keyboard._pressed.clear();
    _keyboard._released.clear();
  }

  void Input::OnKeyboardEvent(WindowHandle window, Key key,
                              KeyState state) noexcept
  {
    switch (state)
    {
      case KeyState::Pressed:  _keyboard._pressed.emplace(key); break;
      case KeyState::Held:     _keyboard._held.emplace(key); break;
      case KeyState::Released: _keyboard._released.emplace(key); break;
    }

    _events->Enqueue(CreateUnique<Events::KeyboardEvent>(window, key, state));
  }

  void Input::OnMouseButtonEvent(WindowHandle window, MouseButton btn,
                                 MouseButtonState state) noexcept
  {
    switch (state)
    {
      case MouseButtonState::Pressed:  _mouse._pressed |= btn; break;
      case MouseButtonState::Released: _mouse._released |= btn; break;
    }

    _events->Enqueue(CreateUnique<Events::MouseButtonEvent>(window, btn, state));
  }

  void Input::OnMouseMoveEvent(WindowHandle window, float dx, float dy, float clientX,
                               float clientY) noexcept
  {
    _mouse._deltaX += dx;
    _mouse._deltaY += dy;
    _mouse._clientX = clientX;
    _mouse._clientY = clientY;

    _events->Enqueue(CreateUnique<Events::MouseMoveEvent>(window, dx, dy, clientX, clientY));
  }

  void Input::OnMouseScrollEvent(WindowHandle window, float delta) noexcept
  {
    _mouse._scrollDelta += delta;
    _events->Enqueue(CreateUnique<Events::MouseScrollEvent>(window, delta));
  }
}