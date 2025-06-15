#include "Krystal.Platform/Events.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Events/Event.hpp"
#include "Krystal.Platform/Keys.hpp"
#include "Krystal.Platform/MouseButtons.hpp"

namespace Krys::Platform
{
#pragma region QuitEvent

  QuitEvent::QuitEvent(NativeHandle handle) noexcept : _handle(handle)
  {
  }

  NativeHandle QuitEvent::GetNativeHandle() const noexcept
  {
    return _handle;
  }

#pragma endregion

#pragma region KeyboardEvent

  KeyboardEvent::KeyboardEvent(const Key key, const KeyState state) noexcept
      : Event(), _key(key), _state(state)
  {
  }

  Key KeyboardEvent::GetKey() const noexcept
  {
    return _key;
  }

  KeyState KeyboardEvent::GetState() const noexcept
  {
    return _state;
  }

#pragma endregion

#pragma region MouseButtonEvent

  MouseButtonEvent::MouseButtonEvent(const MouseButton button, const MouseButtonState state) noexcept
      : Event(), _button(button), _state(state)
  {
  }

  MouseButton MouseButtonEvent::GetButton() const noexcept
  {
    return _button;
  }

  MouseButtonState MouseButtonEvent::GetState() const noexcept
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

#pragma endregion

#pragma region MouseMoveEvent

  MouseMoveEvent::MouseMoveEvent(const float deltaX, const float deltaY, const float clientX,
                                 const float clientY) noexcept
      : Event(), _deltaX(deltaX), _deltaY(deltaY), _clientX(clientX), _clientY(clientY)
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

  float MouseMoveEvent::GetClientX() const noexcept
  {
    return _clientX;
  }

  float MouseMoveEvent::GetClientY() const noexcept
  {
    return _clientY;
  }

#pragma endregion

#pragma region ScrollWheelEvent

  ScrollWheelEvent::ScrollWheelEvent(const float delta) noexcept : Event(), _delta(delta)
  {
  }

  float ScrollWheelEvent::Delta() const noexcept
  {
    return _delta;
  }

#pragma endregion
}
