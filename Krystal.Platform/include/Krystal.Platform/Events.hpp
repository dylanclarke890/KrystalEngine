#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Events/Event.hpp"
#include "Krystal.Platform/Keys.hpp"
#include "Krystal.Platform/MouseButtons.hpp"

namespace Krys::Platform
{
  /// @brief Represents an attempt to close the application.
  ///
  /// A `QuitEvent` is raised automatically in response to common exit requests,
  /// such as pressing the close button or using keyboard shortcuts like ALT + F4 (on Windows).
  class QuitEvent : public Event
  {
  public:
    KRYS_EVENT_CLASS_TYPE("quit-event")

    /// @brief Constructs a `QuitEvent`.
    /// @param handle The native handle of the window that requested the quit event.
    QuitEvent(NativeHandle handle) noexcept;

    /// @brief Gets the native handle of the window that requested the quit event.
    /// @returns The native handle of the window that requested the quit event.
    NO_DISCARD NativeHandle GetNativeHandle() const noexcept;

  private:
    NativeHandle _handle;
  };

  /// @brief Represents an interaction with a keyboard.
  class KeyboardEvent : public Event
  {
  public:
    KRYS_EVENT_CLASS_TYPE("keyboard-event")

    /// @brief Constructs a `KeyboardEvent`.
    /// @param button The key involved in the interaction.
    /// @param state The key's state during the interaction.
    KeyboardEvent(const Key key, const KeyState state) noexcept;

    /// @brief Gets the key involved in the interaction.
    NO_DISCARD Key GetKey() const noexcept;

    /// @brief Gets the state of the key.
    NO_DISCARD KeyState GetState() const noexcept;

  private:
    Key _key;
    KeyState _state;
  };

  /// @brief Represents a mouse button interaction.
  class MouseButtonEvent : public Event
  {
  public:
    KRYS_EVENT_CLASS_TYPE("mouse-button-event")

    /// @brief Constructs a `MouseButtonEvent`.
    /// @param button The mouse button involved in the interaction.
    /// @param state The button's state during the interaction.
    MouseButtonEvent(const MouseButton button, const MouseButtonState state) noexcept;

    /// @brief Gets the mouse button involved in the interaction.
    NO_DISCARD MouseButton GetButton() const noexcept;

    /// @brief Gets the state of the mouse button.
    NO_DISCARD MouseButtonState GetState() const noexcept;

    /// @brief Checks if the button is pressed.
    /// @returns `true` if the button is in the state `MouseButtonState::Pressed`.
    NO_DISCARD bool IsPressed() const noexcept;

    /// @brief Checks if the button was released.
    /// @returns `true` if the button is in the state `MouseButtonState::Released`.
    NO_DISCARD bool WasReleased() const noexcept;

  private:
    MouseButton _button;
    MouseButtonState _state;
  };

  /// @brief Represents a mouse movement.
  class MouseMoveEvent : public Event
  {
  public:
    KRYS_EVENT_CLASS_TYPE("mouse-move-event")

    /// @brief Constructs a `MouseMoveEvent`.
    /// @param deltaX The amount the cursor moved along the x-axis since the last event.
    /// @param deltaY The amount the cursor moved along the y-axis since the last event.
    /// @param clientX The x-coordinate of the cursor in client space.
    /// @param clientY The y-coordinate of the cursor in client space.
    MouseMoveEvent(const float deltaX, const float deltaY, const float clientX, const float clientY) noexcept;

    /// @brief Gets the amount the cursor has moved along the x-axis since the last event.
    NO_DISCARD float DeltaX() const noexcept;

    /// @brief Gets the amount the cursor has moved along the y-axis since the last event.
    NO_DISCARD float DeltaY() const noexcept;

    /// @brief Gets the x-coordinate of the cursor in client space.
    /// @note The origin is the top-left corner of the window, minus the title bar.
    NO_DISCARD float GetClientX() const noexcept;

    /// @brief Gets the y-coordinate of the cursor in client space.
    /// @note The origin is the top-left corner of the window, minus the title bar.
    NO_DISCARD float GetClientY() const noexcept;

  private:
    float _deltaX, _deltaY;
    float _clientX, _clientY;
  };

  /// @brief Represents a user interaction with a mouse scroll wheel.
  class ScrollWheelEvent : public Event
  {
  public:
    KRYS_EVENT_CLASS_TYPE("scroll-wheel-event")

    /// @brief Constructs a `ScrollWheelEvent`.
    /// @param delta The amount the scroll wheel has moved since the last event. Positive values MUST indicate
    /// scrolling forward and negative values MUST indicate scrolling backwards.
    ScrollWheelEvent(const float delta) noexcept;

    /// @brief Gets the scroll delta since the last event. Positive values indicate scrolling forward (away
    /// from the user), and negative values indicate scrolling backward (toward the user).
    NO_DISCARD float Delta() const noexcept;

  private:
    float _delta;
  };
}