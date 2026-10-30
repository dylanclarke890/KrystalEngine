#include "Krystal.PAL/Input.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"

namespace krys::pal
{
  Input::Input(EventManager *events, UniquePtr<GamepadBackend> gamepads) noexcept
      : _keyboard(), _mouse(), _events(events), _gamepadBackend(krys::move(gamepads))
  {
    if (_gamepadBackend)
    {
      _gamepads.resize(_gamepadBackend->SlotCount());
      for (uint32 slot = 0; slot < _gamepads.size(); ++slot)
      {
        _gamepads[slot]._slot = slot;
      }
    }
  }

  Span<const Gamepad> Input::GetGamepads() const noexcept
  {
    return _gamepads;
  }

  bool Input::SetGamepadRumble(uint32 slot, float strong, float weak) noexcept
  {
    if (!_gamepadBackend || slot >= _gamepads.size() || !_gamepads[slot]._connected)
    {
      return false;
    }
    return _gamepadBackend->SetRumble(slot, strong, weak);
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
    _mouse._horizontalScrollDelta = 0;
    _mouse._held |= _mouse._pressed;
    _mouse._held &= ~_mouse._released;
    _mouse._pressed = MouseButton::None;
    _mouse._released = MouseButton::None;

    for (const auto &key : _keyboard._pressed)
      _keyboard._held.emplace(key);

    for (const auto &key : _keyboard._released)
      _keyboard._held.erase(key);

    _keyboard._pressed.clear();
    _keyboard._released.clear();

    for (auto &pad : _gamepads)
    {
      pad._held.Add(pad._pressed);
      pad._held.Remove(pad._released);
      pad._pressed = {};
      pad._released = {};
    }
    PollGamepads();
  }

  void Input::PollGamepads() noexcept
  {
    if (!_gamepadBackend)
    {
      return;
    }

    auto now = nanoseconds(MonotonicTime::Now());
    bool scan = !_lastGamepadScan || now - *_lastGamepadScan >= GamepadScanInterval;
    if (scan)
    {
      _lastGamepadScan = now;
    }

    for (auto &pad : _gamepads)
    {
      if (!pad._connected && !scan)
      {
        continue;
      }

      auto reading = _gamepadBackend->Read(pad._slot);
      if (!reading)
      {
        if (pad._connected)
        {
          // A pad that goes away lets go of its buttons, without events for them.
          auto slot = pad._slot;
          pad = Gamepad();
          pad._slot = slot;
          _events->Enqueue(CreateUnique<GamepadConnectionEvent>(slot, false));
        }
        continue;
      }

      bool connected = !pad._connected;
      if (connected)
      {
        pad._connected = true;
        pad._name = _gamepadBackend->Name(pad._slot);
        _events->Enqueue(CreateUnique<GamepadConnectionEvent>(pad._slot, true));
      }
      if (connected || reading->PacketNumber != pad._packetNumber)
      {
        pad._packetNumber = reading->PacketNumber;
        pad._lastChanged = now;
      }

      pad._buttons = reading->Buttons;
      pad._axes = reading->Axes;
      for (size_t i = 0; i < GamepadButtonCount; ++i)
      {
        auto button = static_cast<GamepadButton>(i);
        bool down = Gamepad::IsPressedValue(button, pad._buttons[i]);
        bool wasDown = pad._held.Contains(button);
        if (down && !wasDown)
        {
          pad._pressed.Add(button);
          _events->Enqueue(CreateUnique<GamepadButtonEvent>(pad._slot, button, GamepadButtonState::Pressed));
        }
        else if (!down && wasDown)
        {
          pad._released.Add(button);
          _events->Enqueue(CreateUnique<GamepadButtonEvent>(pad._slot, button, GamepadButtonState::Released));
        }
      }
    }
  }

  void Input::OnKeyboardEvent(WindowHandle window, Key key, KeyState state, KeyModifiers modifiers,
                              const KeyText &text) noexcept
  {
    switch (state)
    {
      case KeyState::Pressed:  _keyboard._pressed.emplace(key); break;
      case KeyState::Held:     _keyboard._held.emplace(key); break;
      case KeyState::Released: _keyboard._released.emplace(key); break;
    }

    _events->Enqueue(CreateUnique<KeyboardEvent>(window, key, state, modifiers, text));
  }

  void Input::OnTextEvent(WindowHandle window, const utf8_string &text) noexcept
  {
    _events->Enqueue(CreateUnique<TextInputEvent>(window, text));
  }

  void Input::OnCompositionEvent(WindowHandle window, const CompositionInput &input) noexcept
  {
    _events->Enqueue(CreateUnique<CompositionInputEvent>(window, input));
  }

  void Input::OnMouseButtonEvent(WindowHandle window, MouseButton btn, MouseButtonState state,
                                 KeyModifiers modifiers) noexcept
  {
    switch (state)
    {
      case MouseButtonState::Pressed:  _mouse._pressed |= btn; break;
      case MouseButtonState::Released: _mouse._released |= btn; break;
    }

    _events->Enqueue(CreateUnique<MouseButtonEvent>(window, btn, state, modifiers));
  }

  void Input::OnMouseMoveEvent(WindowHandle window, float dx, float dy, float clientX, float clientY,
                               KeyModifiers modifiers) noexcept
  {
    _mouse._deltaX += dx;
    _mouse._deltaY += dy;
    _mouse._clientX = clientX;
    _mouse._clientY = clientY;

    _events->Enqueue(CreateUnique<MouseMoveEvent>(window, dx, dy, clientX, clientY, modifiers));
  }

  void Input::OnLockedMouseMoveEvent(WindowHandle window, float dx, float dy, float rawDx, float rawDy,
                                     KeyModifiers modifiers) noexcept
  {
    // The cursor doesn't move while the pointer is locked.
    _mouse._deltaX += rawDx;
    _mouse._deltaY += rawDy;

    _events->Enqueue(CreateUnique<LockedMouseMoveEvent>(window, dx, dy, rawDx, rawDy, modifiers));
  }

  void Input::OnMouseScrollEvent(WindowHandle window, float deltaX, float deltaY, float clientX,
                                 float clientY, KeyModifiers modifiers) noexcept
  {
    _mouse._scrollDelta += deltaY;
    _mouse._horizontalScrollDelta += deltaX;
    _mouse._clientX = clientX;
    _mouse._clientY = clientY;
    _events->Enqueue(CreateUnique<MouseScrollEvent>(window, deltaX, deltaY, clientX, clientY, modifiers));
  }

  void Input::OnPointerEvent(WindowHandle window, const PointerInput &input) noexcept
  {
    _events->Enqueue(CreateUnique<PointerInputEvent>(window, input));
  }

  void Input::OnDragEvent(WindowHandle window, const DragInput &input) noexcept
  {
    _events->Enqueue(CreateUnique<DragInputEvent>(window, input));
  }
}