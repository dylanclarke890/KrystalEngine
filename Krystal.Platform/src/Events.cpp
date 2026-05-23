#include "Krystal.Platform/Events.hpp"
#include "Krystal.Lib/Events/Event.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Platform/Keys.hpp"
#include "Krystal.Platform/MouseButtons.hpp"

namespace Krys::Events
{
#pragma region KeyboardEvent

  KeyboardEvent::KeyboardEvent(Platform::WindowHandle window, const Platform::Key key,
                               const Platform::KeyState state) noexcept
      : Event(), _window(window), _key(key), _state(state)
  {
  }

  Platform::Key KeyboardEvent::Key() const noexcept
  {
    return _key;
  }

  Platform::KeyState KeyboardEvent::State() const noexcept
  {
    return _state;
  }

  Platform::WindowHandle KeyboardEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region MouseButtonEvent

  MouseButtonEvent::MouseButtonEvent(Platform::WindowHandle window, const Platform::MouseButton button,
                                     const Platform::MouseButtonState state) noexcept
      : Event(), _window(window), _button(button), _state(state)
  {
  }

  Platform::MouseButton MouseButtonEvent::Button() const noexcept
  {
    return _button;
  }

  Platform::MouseButtonState MouseButtonEvent::State() const noexcept
  {
    return _state;
  }

  bool MouseButtonEvent::IsPressed() const noexcept
  {
    return _state == Platform::MouseButtonState::Pressed;
  }

  bool MouseButtonEvent::WasReleased() const noexcept
  {
    return _state == Platform::MouseButtonState::Released;
  }

  Platform::WindowHandle MouseButtonEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region MouseMoveEvent

  MouseMoveEvent::MouseMoveEvent(Platform::WindowHandle window, const float deltaX, const float deltaY,
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

  Platform::WindowHandle MouseMoveEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region MouseScrollEvent

  MouseScrollEvent::MouseScrollEvent(Platform::WindowHandle window, const float delta) noexcept
      : Event(), _window(window), _delta(delta)
  {
  }

  float MouseScrollEvent::Delta() const noexcept
  {
    return _delta;
  }

  Platform::WindowHandle MouseScrollEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region WindowResizeEvent

  WindowResizeEvent::WindowResizeEvent(Platform::WindowHandle window, const uint32 width,
                                       const uint32 height) noexcept
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

  Platform::WindowHandle WindowResizeEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region WindowMinimiseEvent

  WindowMinimiseEvent::WindowMinimiseEvent(Platform::WindowHandle window) noexcept : Event(), _window(window)
  {
  }

  Platform::WindowHandle WindowMinimiseEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region WindowRestoreEvent

  WindowRestoreEvent::WindowRestoreEvent(Platform::WindowHandle window) noexcept : Event(), _window(window)
  {
  }

  Platform::WindowHandle WindowRestoreEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region WindowCloseEvent

  WindowCloseEvent::WindowCloseEvent(Platform::WindowHandle window) noexcept : _window(window)
  {
  }

  Platform::WindowHandle WindowCloseEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region WindowCloseEvent

  WindowDPIChangeEvent::WindowDPIChangeEvent(Platform::WindowHandle window, int dpi) noexcept
      : _window(window), _dpi(dpi)
  {
  }

  int WindowDPIChangeEvent::DPI() const noexcept
  {
    return _dpi;
  }

  Platform::WindowHandle WindowDPIChangeEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion
}
