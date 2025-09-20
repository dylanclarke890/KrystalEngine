#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Events/Event.hpp"
#include "Krystal.Platform/IWindow.hpp"
#include "Krystal.Platform/Keys.hpp"
#include "Krystal.Platform/MouseButtons.hpp"

namespace Krys::Events
{
  /// @brief Represents an interaction with a keyboard.
  class KeyboardEvent : public Event
  {
  public:
    KRYS_EVENT_CLASS_TYPE("keyboard-event")

    /// @brief Constructs a `KeyboardEvent`.
    /// @param button The key involved in the interaction.
    /// @param state The key's state during the interaction.
    KeyboardEvent(Platform::WindowHandle window, const Platform::Key key,
                  const Platform::KeyState state) noexcept;

    /// @brief Gets the key involved in the interaction.
    NO_DISCARD Platform::Key Key() const noexcept;

    /// @brief Gets the state of the key.
    NO_DISCARD Platform::KeyState State() const noexcept;

    NO_DISCARD Platform::WindowHandle GetWindowHandle() const noexcept;

  private:
    Platform::WindowHandle _window;
    Platform::Key _key;
    Platform::KeyState _state;
  };

  /// @brief Represents a mouse button interaction.
  class MouseButtonEvent : public Event
  {
  public:
    KRYS_EVENT_CLASS_TYPE("mouse-button-event")

    /// @brief Constructs a `MouseButtonEvent`.
    /// @param button The mouse button involved in the interaction.
    /// @param state The button's state during the interaction.
    MouseButtonEvent(Platform::WindowHandle window, const Platform::MouseButton button,
                     const Platform::MouseButtonState state) noexcept;

    /// @brief Gets the mouse button involved in the interaction.
    NO_DISCARD Platform::MouseButton Button() const noexcept;

    /// @brief Gets the state of the mouse button.
    NO_DISCARD Platform::MouseButtonState State() const noexcept;

    /// @brief Checks if the button is pressed.
    /// @returns `true` if the button is in the state `MouseButtonState::Pressed`.
    NO_DISCARD bool IsPressed() const noexcept;

    /// @brief Checks if the button was released.
    /// @returns `true` if the button is in the state `MouseButtonState::Released`.
    NO_DISCARD bool WasReleased() const noexcept;

    NO_DISCARD Platform::WindowHandle GetWindowHandle() const noexcept;

  private:
    Platform::WindowHandle _window;
    Platform::MouseButton _button;
    Platform::MouseButtonState _state;
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
    MouseMoveEvent(Platform::WindowHandle window, const float deltaX, const float deltaY, const float clientX,
                   const float clientY) noexcept;

    /// @brief Gets the amount the cursor has moved along the x-axis since the last event.
    NO_DISCARD float DeltaX() const noexcept;

    /// @brief Gets the amount the cursor has moved along the y-axis since the last event.
    NO_DISCARD float DeltaY() const noexcept;

    /// @brief Gets the x-coordinate of the cursor in client space.
    /// @note The origin is the top-left corner of the window, minus the title bar.
    NO_DISCARD float ClientX() const noexcept;

    /// @brief Gets the y-coordinate of the cursor in client space.
    /// @note The origin is the top-left corner of the window, minus the title bar.
    NO_DISCARD float ClientY() const noexcept;

    NO_DISCARD Platform::WindowHandle GetWindowHandle() const noexcept;

  private:
    Platform::WindowHandle _window;
    float _deltaX, _deltaY;
    float _clientX, _clientY;
  };

  /// @brief Represents a user interaction with a mouse scroll wheel.
  class MouseScrollEvent : public Event
  {
  public:
    KRYS_EVENT_CLASS_TYPE("mouse-scroll-event")

    /// @brief Constructs a `ScrollWheelEvent`.
    /// @param delta The amount the scroll wheel has moved since the last event. Positive values MUST indicate
    /// scrolling forward and negative values MUST indicate scrolling backwards.
    MouseScrollEvent(Platform::WindowHandle window, const float delta) noexcept;

    /// @brief Gets the scroll delta since the last event. Positive values indicate scrolling forward (away
    /// from the user), and negative values indicate scrolling backward (toward the user).
    NO_DISCARD float Delta() const noexcept;

    NO_DISCARD Platform::WindowHandle GetWindowHandle() const noexcept;

  private:
    Platform::WindowHandle _window;
    float _delta;
  };

  class WindowResizeEvent : public Event
  {
  public:
    KRYS_EVENT_CLASS_TYPE("window-resize-event")

    /// @brief Constructs a `WindowResizeEvent`.
    /// @param width The new width of the window.
    /// @param height The new height of the window.
    WindowResizeEvent(Platform::WindowHandle window, const uint32 width, const uint32 height) noexcept;

    NO_DISCARD uint32 Width() const noexcept;
    NO_DISCARD uint32 Height() const noexcept;

    NO_DISCARD Platform::WindowHandle GetWindowHandle() const noexcept;

  private:
    Platform::WindowHandle _window;
    uint32 _width, _height;
  };

  class WindowMinimiseEvent : public Event
  {
  public:
    KRYS_EVENT_CLASS_TYPE("window-minimise-event")

    /// @brief Constructs a `WindowMinimiseEvent`.
    WindowMinimiseEvent(Platform::WindowHandle window) noexcept;

    NO_DISCARD Platform::WindowHandle GetWindowHandle() const noexcept;

  private:
    Platform::WindowHandle _window;
  };

  class WindowRestoreEvent : public Event
  {
  public:
    KRYS_EVENT_CLASS_TYPE("window-restore-event")

    /// @brief Constructs a `WindowRestoreEvent`.
    WindowRestoreEvent(Platform::WindowHandle window) noexcept;

    NO_DISCARD Platform::WindowHandle GetWindowHandle() const noexcept;

  private:
    Platform::WindowHandle _window;
  };

  /// @brief Represents an attempt to close the application.
  ///
  /// A `WindowCloseEvent` is raised automatically in response to common exit requests,
  /// such as pressing the close button or using keyboard shortcuts like ALT + F4 (on Windows).
  class WindowCloseEvent : public Event
  {
  public:
    KRYS_EVENT_CLASS_TYPE("window-close-event")

    /// @brief Constructs a `WindowCloseEvent`.
    /// @param handle The native handle of the window that requested the quit event.
    WindowCloseEvent(Platform::WindowHandle window) noexcept;

    NO_DISCARD Platform::WindowHandle GetWindowHandle() const noexcept;

  private:
    Platform::WindowHandle _window;
  };
}