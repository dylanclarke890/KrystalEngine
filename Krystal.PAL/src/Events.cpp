#include "Krystal.PAL/Events.hpp"
#include "Krystal.Core/Events/Event.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.PAL/Keys.hpp"
#include "Krystal.PAL/MouseButtons.hpp"

namespace krys::pal
{
#pragma region KeyboardEvent

  KeyboardEvent::KeyboardEvent(WindowHandle window, const krys::pal::Key key, const KeyState state) noexcept
      : Event(), _window(window), _key(key), _state(state)
  {
  }

  Key KeyboardEvent::Key() const noexcept
  {
    return _key;
  }

  KeyState KeyboardEvent::State() const noexcept
  {
    return _state;
  }

  WindowHandle KeyboardEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region MouseButtonEvent

  MouseButtonEvent::MouseButtonEvent(WindowHandle window, const MouseButton button,
                                     const MouseButtonState state) noexcept
      : Event(), _window(window), _button(button), _state(state)
  {
  }

  MouseButton MouseButtonEvent::Button() const noexcept
  {
    return _button;
  }

  MouseButtonState MouseButtonEvent::State() const noexcept
  {
    return _state;
  }

  bool MouseButtonEvent::IsPressed() const noexcept
  {
    return _state == MouseButtonState::Pressed;
  }

  bool MouseButtonEvent::WasReleased() const noexcept
  {
    return _state == MouseButtonState::Released;
  }

  WindowHandle MouseButtonEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region MouseMoveEvent

  MouseMoveEvent::MouseMoveEvent(WindowHandle window, const float deltaX, const float deltaY,
                                 const float clientX, const float clientY) noexcept
      : Event(), _window(window), _deltaX(deltaX), _deltaY(deltaY), _clientX(clientX), _clientY(clientY)
  {
  }

  float MouseMoveEvent::DeltaX() const noexcept
  {
    return _deltaX;
  }

  float MouseMoveEvent::DeltaY() const noexcept
  {
    return _deltaY;
  }

  float MouseMoveEvent::ClientX() const noexcept
  {
    return _clientX;
  }

  float MouseMoveEvent::ClientY() const noexcept
  {
    return _clientY;
  }

  WindowHandle MouseMoveEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region MouseScrollEvent

  MouseScrollEvent::MouseScrollEvent(WindowHandle window, const float delta) noexcept
      : Event(), _window(window), _delta(delta)
  {
  }

  float MouseScrollEvent::Delta() const noexcept
  {
    return _delta;
  }

  WindowHandle MouseScrollEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region WindowResizeEvent

  WindowResizeEvent::WindowResizeEvent(WindowHandle window, const uint32 width, const uint32 height) noexcept
      : Event(), _window(window), _width(width), _height(height)
  {
  }

  uint32 WindowResizeEvent::Width() const noexcept
  {
    return _width;
  }

  uint32 WindowResizeEvent::Height() const noexcept
  {
    return _height;
  }

  WindowHandle WindowResizeEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region WindowMinimiseEvent

  WindowMinimiseEvent::WindowMinimiseEvent(WindowHandle window) noexcept : Event(), _window(window)
  {
  }

  WindowHandle WindowMinimiseEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region WindowRestoreEvent

  WindowRestoreEvent::WindowRestoreEvent(WindowHandle window) noexcept : Event(), _window(window)
  {
  }

  WindowHandle WindowRestoreEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region WindowCloseEvent

  WindowCloseEvent::WindowCloseEvent(WindowHandle window) noexcept : _window(window)
  {
  }

  WindowHandle WindowCloseEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region WindowCloseEvent

  WindowDPIChangeEvent::WindowDPIChangeEvent(WindowHandle window, int dpi) noexcept
      : _window(window), _dpi(dpi)
  {
  }

  int WindowDPIChangeEvent::DPI() const noexcept
  {
    return _dpi;
  }

  WindowHandle WindowDPIChangeEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion
}
