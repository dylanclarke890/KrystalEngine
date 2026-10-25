#include "Krystal.PAL/Events.hpp"
#include "Krystal.Core/Events/Event.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.PAL/Keys.hpp"
#include "Krystal.PAL/MouseButtons.hpp"

namespace krys::pal
{
#pragma region KeyboardEvent

  KeyboardEvent::KeyboardEvent(WindowHandle window, const krys::pal::Key key, const KeyState state,
                               const KeyModifiers modifiers, KeyText text) noexcept
      : Event(), _window(window), _key(key), _state(state), _modifiers(modifiers), _text(krys::move(text))
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

  KeyModifiers KeyboardEvent::Modifiers() const noexcept
  {
    return _modifiers;
  }

  const KeyText &KeyboardEvent::Text() const noexcept
  {
    return _text;
  }

  WindowHandle KeyboardEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region PointerInputEvent

  PointerInputEvent::PointerInputEvent(WindowHandle window, const PointerInput &input) noexcept
      : Event(), _window(window), _input(input)
  {
  }

  const PointerInput &PointerInputEvent::Input() const noexcept
  {
    return _input;
  }

  WindowHandle PointerInputEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region MouseButtonEvent

  MouseButtonEvent::MouseButtonEvent(WindowHandle window, const MouseButton button,
                                     const MouseButtonState state, const KeyModifiers modifiers) noexcept
      : Event(), _window(window), _button(button), _state(state), _modifiers(modifiers)
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

  KeyModifiers MouseButtonEvent::Modifiers() const noexcept
  {
    return _modifiers;
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
                                 const float clientX, const float clientY,
                                 const KeyModifiers modifiers) noexcept
      : Event(), _window(window), _deltaX(deltaX), _deltaY(deltaY), _clientX(clientX), _clientY(clientY),
        _modifiers(modifiers)
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

  KeyModifiers MouseMoveEvent::Modifiers() const noexcept
  {
    return _modifiers;
  }

  WindowHandle MouseMoveEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion

#pragma region MouseScrollEvent

  MouseScrollEvent::MouseScrollEvent(WindowHandle window, const float deltaX, const float deltaY,
                                     const float clientX, const float clientY,
                                     const KeyModifiers modifiers) noexcept
      : Event(), _window(window), _deltaX(deltaX), _deltaY(deltaY), _clientX(clientX), _clientY(clientY),
        _modifiers(modifiers)
  {
  }

  float MouseScrollEvent::DeltaX() const noexcept
  {
    return _deltaX;
  }

  float MouseScrollEvent::DeltaY() const noexcept
  {
    return _deltaY;
  }

  float MouseScrollEvent::ClientX() const noexcept
  {
    return _clientX;
  }

  float MouseScrollEvent::ClientY() const noexcept
  {
    return _clientY;
  }

  KeyModifiers MouseScrollEvent::Modifiers() const noexcept
  {
    return _modifiers;
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

#pragma region WindowFocusEvent

  WindowFocusEvent::WindowFocusEvent(WindowHandle window, const bool focused) noexcept
      : _window(window), _focused(focused)
  {
  }

  bool WindowFocusEvent::Focused() const noexcept
  {
    return _focused;
  }

  WindowHandle WindowFocusEvent::GetWindowHandle() const noexcept
  {
    return _window;
  }

#pragma endregion
}
