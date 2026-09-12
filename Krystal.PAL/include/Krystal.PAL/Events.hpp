#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Events/Event.hpp"
#include "Krystal.PAL/IWindow.hpp"
#include "Krystal.PAL/Keys.hpp"
#include "Krystal.PAL/MouseButtons.hpp"

namespace krys::pal
{
  /// @brief Represents an interaction with a keyboard.
  class KeyboardEvent : public Event
  {
  private:
    WindowHandle _window;
    Key _key;
    KeyState _state;

  public:
    KRYS_EVENT_CLASS_TYPE("keyboard-event")

    /// @brief Constructs a `KeyboardEvent`.
    /// @param button The key involved in the interaction.
    /// @param state The key's state during the interaction.
    KeyboardEvent(WindowHandle window, const Key key, const KeyState state) noexcept;

    /// @brief Gets the key involved in the interaction.
    KRYS_NODISCARD Key Key() const noexcept;

    /// @brief Gets the state of the key.
    KRYS_NODISCARD KeyState State() const noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };

  /// @brief Represents a mouse button interaction.
  class MouseButtonEvent : public Event
  {
  private:
    WindowHandle _window;
    MouseButton _button;
    MouseButtonState _state;

  public:
    KRYS_EVENT_CLASS_TYPE("mouse-button-event")

    /// @brief Constructs a `MouseButtonEvent`.
    /// @param button The mouse button involved in the interaction.
    /// @param state The button's state during the interaction.
    MouseButtonEvent(WindowHandle window, const MouseButton button, const MouseButtonState state) noexcept;

    /// @brief Gets the mouse button involved in the interaction.
    KRYS_NODISCARD MouseButton Button() const noexcept;

    /// @brief Gets the state of the mouse button.
    KRYS_NODISCARD MouseButtonState State() const noexcept;

    /// @brief Checks if the button is pressed.
    /// @returns `true` if the button is in the state `MouseButtonState::Pressed`.
    KRYS_NODISCARD bool IsPressed() const noexcept;

    /// @brief Checks if the button was released.
    /// @returns `true` if the button is in the state `MouseButtonState::Released`.
    KRYS_NODISCARD bool WasReleased() const noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };

  /// @brief Represents a mouse movement.
  class MouseMoveEvent : public Event
  {
  private:
    WindowHandle _window;
    float _deltaX, _deltaY;
    float _clientX, _clientY;

  public:
    KRYS_EVENT_CLASS_TYPE("mouse-move-event")

    /// @brief Constructs a `MouseMoveEvent`.
    /// @param deltaX The amount the cursor moved along the x-axis since the last event.
    /// @param deltaY The amount the cursor moved along the y-axis since the last event.
    /// @param clientX The x-coordinate of the cursor in client space.
    /// @param clientY The y-coordinate of the cursor in client space.
    MouseMoveEvent(WindowHandle window, const float deltaX, const float deltaY, const float clientX,
                   const float clientY) noexcept;

    /// @brief Gets the amount the cursor has moved along the x-axis since the last event.
    KRYS_NODISCARD float DeltaX() const noexcept;

    /// @brief Gets the amount the cursor has moved along the y-axis since the last event.
    KRYS_NODISCARD float DeltaY() const noexcept;

    /// @brief Gets the x-coordinate of the cursor in client space.
    /// @note The origin is the top-left corner of the window, minus the title bar.
    KRYS_NODISCARD float ClientX() const noexcept;

    /// @brief Gets the y-coordinate of the cursor in client space.
    /// @note The origin is the top-left corner of the window, minus the title bar.
    KRYS_NODISCARD float ClientY() const noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };

  /// @brief Represents a user interaction with a mouse scroll wheel.
  class MouseScrollEvent : public Event
  {
  private:
    WindowHandle _window;
    float _delta;

  public:
    KRYS_EVENT_CLASS_TYPE("mouse-scroll-event")

    /// @brief Constructs a `ScrollWheelEvent`.
    /// @param delta The amount the scroll wheel has moved since the last event. Positive values MUST indicate
    /// scrolling forward and negative values MUST indicate scrolling backwards.
    MouseScrollEvent(WindowHandle window, const float delta) noexcept;

    /// @brief Gets the scroll delta since the last event. Positive values indicate scrolling forward (away
    /// from the user), and negative values indicate scrolling backward (toward the user).
    KRYS_NODISCARD float Delta() const noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };

  class WindowResizeEvent : public Event
  {
  private:
    WindowHandle _window;
    uint32 _width, _height;

  public:
    KRYS_EVENT_CLASS_TYPE("window-resize-event")

    /// @brief Constructs a `WindowResizeEvent`.
    /// @param width The new width of the window.
    /// @param height The new height of the window.
    WindowResizeEvent(WindowHandle window, const uint32 width, const uint32 height) noexcept;

    KRYS_NODISCARD uint32 Width() const noexcept;

    KRYS_NODISCARD uint32 Height() const noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };

  class WindowMinimiseEvent : public Event
  {
  private:
    WindowHandle _window;

  public:
    KRYS_EVENT_CLASS_TYPE("window-minimise-event")

    /// @brief Constructs a `WindowMinimiseEvent`.
    WindowMinimiseEvent(WindowHandle window) noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };

  class WindowRestoreEvent : public Event
  {
  private:
    WindowHandle _window;

  public:
    KRYS_EVENT_CLASS_TYPE("window-restore-event")

    /// @brief Constructs a `WindowRestoreEvent`.
    WindowRestoreEvent(WindowHandle window) noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };

  /// @brief Represents an attempt to close the application.
  ///
  /// A `WindowCloseEvent` is raised automatically in response to common exit requests,
  /// such as pressing the close button or using keyboard shortcuts like ALT + F4 (on Windows).
  class WindowCloseEvent : public Event
  {
  private:
    WindowHandle _window;

  public:
    KRYS_EVENT_CLASS_TYPE("window-close-event")

    /// @brief Constructs a `WindowCloseEvent`.
    /// @param handle The native handle of the window that requested the quit event.
    WindowCloseEvent(WindowHandle window) noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };

  /// @brief Represents a dpi change, usually due to moving a window between monitors with different dpi
  /// settings.
  class WindowDPIChangeEvent : public Event
  {
  private:
    WindowHandle _window;
    int _dpi;

  public:
    KRYS_EVENT_CLASS_TYPE("window-dpi-change-event")

    /// @brief Constructs a `WindowDPIChangeEvent`.
    /// @param dpi The new dpi of the window.
    WindowDPIChangeEvent(WindowHandle window, const int dpi) noexcept;

    KRYS_NODISCARD int DPI() const noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };
}