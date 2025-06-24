#include "Krystal.Engine/Input.hpp"

namespace Krys::Engine
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
    _mouse._pressed = Platform::MouseButton::None;
    _mouse._released = Platform::MouseButton::None;

    _keyboard._pressed.clear();
    _keyboard._held.clear();
    _keyboard._released.clear();
  }

  // TODO: figure out what to do with the window handle

  void Input::OnKeyboardEvent(Platform::WindowHandle window, Platform::Key key,
                              Platform::KeyState state) noexcept
  {
    switch (state)
    {
      case Platform::KeyState::Pressed:  _keyboard._pressed.emplace(key); break;
      case Platform::KeyState::Held:     _keyboard._held.emplace(key); break;
      case Platform::KeyState::Released: _keyboard._released.emplace(key); break;
    }

    _events->Enqueue(CreateUnique<KeyboardEvent>(window, key, state));
  }

  void Input::OnMouseButtonEvent(Platform::WindowHandle window, Platform::MouseButton btn,
                                 Platform::MouseButtonState state) noexcept
  {
    switch (state)
    {
      case Platform::MouseButtonState::Pressed:  _mouse._pressed |= btn; break;
      case Platform::MouseButtonState::Released: _mouse._released |= btn; break;
    }

    _events->Enqueue(CreateUnique<MouseButtonEvent>(window, btn, state));
  }

  void Input::OnMouseMoveEvent(Platform::WindowHandle window, float dx, float dy, float clientX,
                               float clientY) noexcept
  {
    _mouse._deltaX += dx;
    _mouse._deltaY += dy;
    _mouse._clientX = clientX;
    _mouse._clientY = clientY;

    _events->Enqueue(CreateUnique<MouseMoveEvent>(window, dx, dy, clientX, clientY));
  }

  void Input::OnMouseScrollEvent(Platform::WindowHandle window, float delta) noexcept
  {
    _mouse._scrollDelta += delta;
    _events->Enqueue(CreateUnique<MouseScrollEvent>(window, delta));
  }
}