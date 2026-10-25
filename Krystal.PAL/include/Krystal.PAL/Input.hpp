#pragma once

#include "Krystal.Core/Events/EventManager.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.PAL/Events.hpp"
#include "Krystal.PAL/Gamepad.hpp"
#include "Krystal.PAL/IWindow.hpp"
#include "Krystal.PAL/Keyboard.hpp"
#include "Krystal.PAL/Mouse.hpp"

namespace krys
{
  class Application;
}

namespace krys::pal
{
  class Input
  {
    friend class Application;

  private:
    Keyboard _keyboard;
    Mouse _mouse;
    EventManager *_events;
    UniquePtr<GamepadBackend> _gamepadBackend;
    /// @brief The pad in each of the backend's slots.
    List<Gamepad> _gamepads;
    /// @brief When the empty slots were last checked for pads, which is slow, so done once a second.
    Maybe<nanoseconds> _lastGamepadScan;

  public:
    /// @brief How often empty gamepad slots are checked for a pad.
    constexpr static nanoseconds GamepadScanInterval {1'000'000'000.0};

    /// @param gamepads Where gamepads are read from: the platform's backend unless given one (or null for no
    /// gamepads).
    explicit Input(EventManager *events,
                   UniquePtr<GamepadBackend> gamepads = CreateGamepadBackend()) noexcept;

    ~Input() noexcept = default;

    /// @brief Starts a frame: the last frame's presses and releases become held and released states, and the
    /// gamepads are read (queueing their connections, presses and releases as events).
    void BeginFrame() noexcept;

    const Keyboard &GetKeyboard() const noexcept;

    const Mouse &GetMouse() const noexcept;

    /// @brief Each gamepad slot's state, by slot.
    KRYS_NODISCARD Span<const Gamepad> GetGamepads() const noexcept;

    /// @brief Sets the rumble motors' strengths, from 0 to 1, of the pad in @p slot, until they're set again.
    /// @return Whether there's a pad there that took them.
    bool SetGamepadRumble(uint32 slot, float strong, float weak) noexcept;

  private:
    void OnKeyboardEvent(WindowHandle window, Key key, KeyState state, KeyModifiers modifiers,
                         const KeyText &text) noexcept;

    void OnMouseButtonEvent(WindowHandle window, MouseButton btn, MouseButtonState state,
                            KeyModifiers modifiers) noexcept;

    void OnMouseMoveEvent(WindowHandle window, float dx, float dy, float clientX, float clientY,
                          KeyModifiers modifiers) noexcept;

    void OnLockedMouseMoveEvent(WindowHandle window, float dx, float dy, float rawDx, float rawDy,
                                KeyModifiers modifiers) noexcept;

    void OnMouseScrollEvent(WindowHandle window, float deltaX, float deltaY, float clientX, float clientY,
                            KeyModifiers modifiers) noexcept;

    void OnPointerEvent(WindowHandle window, const PointerInput &input) noexcept;

    void PollGamepads() noexcept;
  };
}
