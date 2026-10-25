#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Events/Event.hpp"
#include "Krystal.PAL/IWindow.hpp"
#include "Krystal.PAL/KeyModifiers.hpp"
#include "Krystal.PAL/Keys.hpp"
#include "Krystal.PAL/MouseButtons.hpp"
#include "Krystal.PAL/Pointer.hpp"

namespace krys::pal
{
  /// @brief Represents an interaction with a keyboard.
  class KeyboardEvent : public Event
  {
  private:
    WindowHandle _window;
    Key _key;
    KeyState _state;
    KeyModifiers _modifiers;
    KeyText _text;

  public:
    KRYS_EVENT_CLASS_TYPE("keyboard-event")

    /// @brief Constructs a `KeyboardEvent`.
    /// @param key The key involved in the interaction.
    /// @param state The key's state during the interaction.
    /// @param modifiers The modifier keys held.
    /// @param text The text the key types, for presses.
    KeyboardEvent(WindowHandle window, const Key key, const KeyState state, const KeyModifiers modifiers = {},
                  KeyText text = {}) noexcept;

    /// @brief Gets the key involved in the interaction.
    KRYS_NODISCARD Key Key() const noexcept;

    /// @brief Gets the state of the key.
    KRYS_NODISCARD KeyState State() const noexcept;

    /// @brief Gets the modifier keys held during the interaction.
    KRYS_NODISCARD KeyModifiers Modifiers() const noexcept;

    /// @brief Gets the text the key types (empty for releases).
    KRYS_NODISCARD const KeyText &Text() const noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };

  /// @brief Represents a mouse button interaction.
  class MouseButtonEvent : public Event
  {
  private:
    WindowHandle _window;
    MouseButton _button;
    MouseButtonState _state;
    KeyModifiers _modifiers;

  public:
    KRYS_EVENT_CLASS_TYPE("mouse-button-event")

    /// @brief Constructs a `MouseButtonEvent`.
    /// @param button The mouse button involved in the interaction.
    /// @param state The button's state during the interaction.
    /// @param modifiers The modifier keys held.
    MouseButtonEvent(WindowHandle window, const MouseButton button, const MouseButtonState state,
                     const KeyModifiers modifiers = {}) noexcept;

    /// @brief Gets the mouse button involved in the interaction.
    KRYS_NODISCARD MouseButton Button() const noexcept;

    /// @brief Gets the state of the mouse button.
    KRYS_NODISCARD MouseButtonState State() const noexcept;

    /// @brief Gets the modifier keys held during the interaction.
    KRYS_NODISCARD KeyModifiers Modifiers() const noexcept;

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
    KeyModifiers _modifiers;

  public:
    KRYS_EVENT_CLASS_TYPE("mouse-move-event")

    /// @brief Constructs a `MouseMoveEvent`.
    /// @param deltaX The amount the cursor moved along the x-axis since the last event.
    /// @param deltaY The amount the cursor moved along the y-axis since the last event.
    /// @param clientX The x-coordinate of the cursor in client space.
    /// @param clientY The y-coordinate of the cursor in client space.
    /// @param modifiers The modifier keys held.
    MouseMoveEvent(WindowHandle window, const float deltaX, const float deltaY, const float clientX,
                   const float clientY, const KeyModifiers modifiers = {}) noexcept;

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

    /// @brief Gets the modifier keys held during the movement.
    KRYS_NODISCARD KeyModifiers Modifiers() const noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };

  /// @brief Represents a mouse movement while the pointer is locked.
  class LockedMouseMoveEvent : public Event
  {
  private:
    WindowHandle _window;
    float _deltaX, _deltaY;
    float _rawDeltaX, _rawDeltaY;
    KeyModifiers _modifiers;

  public:
    KRYS_EVENT_CLASS_TYPE("locked-mouse-move-event")

    /// @brief Constructs a `LockedMouseMoveEvent`.
    /// @param deltaX The cursor's movement along the x-axis since the last event, with the system's
    /// acceleration.
    /// @param deltaY The cursor's movement along the y-axis since the last event, with the system's
    /// acceleration.
    /// @param rawDeltaX The device's movement along the x-axis since the last event, without it.
    /// @param rawDeltaY The device's movement along the y-axis since the last event, without it.
    /// @param modifiers The modifier keys held.
    LockedMouseMoveEvent(WindowHandle window, float deltaX, float deltaY, float rawDeltaX, float rawDeltaY,
                         KeyModifiers modifiers = {}) noexcept;

    KRYS_NODISCARD float DeltaX() const noexcept;

    KRYS_NODISCARD float DeltaY() const noexcept;

    KRYS_NODISCARD float RawDeltaX() const noexcept;

    KRYS_NODISCARD float RawDeltaY() const noexcept;

    KRYS_NODISCARD KeyModifiers Modifiers() const noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };

  /// @brief Represents a user interaction with a mouse scroll wheel (or a touchpad's scrolling).
  class MouseScrollEvent : public Event
  {
  private:
    WindowHandle _window;
    float _deltaX, _deltaY;
    float _clientX, _clientY;
    KeyModifiers _modifiers;

  public:
    KRYS_EVENT_CLASS_TYPE("mouse-scroll-event")

    /// @brief Constructs a `MouseScrollEvent`.
    /// @param deltaX How far the wheel turned sideways since the last event, in notches. Positive values MUST
    /// indicate scrolling right.
    /// @param deltaY How far the wheel turned since the last event, in notches. Positive values MUST indicate
    /// scrolling forward and negative values MUST indicate scrolling backwards.
    /// @param clientX The x-coordinate of the cursor in client space.
    /// @param clientY The y-coordinate of the cursor in client space.
    /// @param modifiers The modifier keys held.
    MouseScrollEvent(WindowHandle window, const float deltaX, const float deltaY, const float clientX,
                     const float clientY, const KeyModifiers modifiers = {}) noexcept;

    /// @brief Gets the sideways turn since the last event, in notches. Positive values indicate scrolling
    /// right.
    KRYS_NODISCARD float DeltaX() const noexcept;

    /// @brief Gets the turn since the last event, in notches. Positive values indicate scrolling forward
    /// (away from the user), and negative values indicate scrolling backward (toward the user).
    /// @note A notch is usually a whole step; touchpads and smooth wheels report fractions of one.
    KRYS_NODISCARD float DeltaY() const noexcept;

    /// @brief Gets the x-coordinate of the cursor in client space.
    KRYS_NODISCARD float ClientX() const noexcept;

    /// @brief Gets the y-coordinate of the cursor in client space.
    KRYS_NODISCARD float ClientY() const noexcept;

    /// @brief Gets the modifier keys held during the scroll.
    KRYS_NODISCARD KeyModifiers Modifiers() const noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };

  /// @brief Represents a pen's or a touch's input.
  class PointerInputEvent : public Event
  {
  private:
    WindowHandle _window;
    PointerInput _input;

  public:
    KRYS_EVENT_CLASS_TYPE("pointer-input-event")

    /// @brief Constructs a `PointerInputEvent`.
    /// @param input The pen's or the touch's input.
    PointerInputEvent(WindowHandle window, const PointerInput &input) noexcept;

    KRYS_NODISCARD const PointerInput &Input() const noexcept;

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

  /// @brief A window gained or lost keyboard focus.
  class WindowFocusEvent : public Event
  {
  private:
    WindowHandle _window;
    bool _focused;

  public:
    KRYS_EVENT_CLASS_TYPE("window-focus-event")

    /// @brief Constructs a `WindowFocusEvent`.
    /// @param focused Whether the window gained focus (rather than lost it).
    WindowFocusEvent(WindowHandle window, const bool focused) noexcept;

    KRYS_NODISCARD bool Focused() const noexcept;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept;
  };
}
