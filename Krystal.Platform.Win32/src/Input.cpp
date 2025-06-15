#include "Krystal.Platform.Win32/Input.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Events/EventManager.hpp"
#include "Krystal.Platform/Events.hpp"
#include "Krystal.Platform/IInput.hpp"
#include "Krystal.Platform/Keys.hpp"
#include "Krystal.Platform/MouseButtons.hpp"

#include <cstring>
#include <windows.h>

namespace Krys::Platform
{
  Unique<IInput> CreateInput(Ptr<EventManager> eventManager) noexcept
  {
    return Unique<IInput>(new Win32Input(eventManager));
  }

  Win32Input::Win32Input(Ptr<EventManager> eventManager) noexcept : _eventManager(eventManager)
  {
  }

  void Win32Input::BeginFrame() noexcept
  {
    ClearState();
  }

  void Win32Input::PollDevices() noexcept
  {
  }

  bool Win32Input::HandleWindowMessage(UINT message, WPARAM wParam, LPARAM lParam, HWND windowHandle) noexcept
  {
#define MOUSE_BUTTON_MESSAGE(btn, btnState)                                                                  \
  const auto button = MouseButton::##btn;                                                                    \
  const auto state = MouseButtonState::##btnState;                                                           \
  OnMouseButtonEvent(button, state);                                                                         \
  _eventManager->Enqueue(CreateUnique<MouseButtonEvent>(button, state));                                     \
  break;

    switch (message)
    {
      case WM_KEYDOWN:
      {
        const auto key = KeyCodeToEngineKey(wParam);
        const auto state = _pressedKeys.contains(wParam) ? KeyState::Held : KeyState::Pressed;
        OnKeyboardEvent(key, state);
        _eventManager->Enqueue(CreateUnique<KeyboardEvent>(key, state));
        _pressedKeys.emplace(wParam);
        break;
      }
      case WM_KEYUP:
      {
        const auto key = KeyCodeToEngineKey(wParam);
        const auto state = KeyState::Released;
        OnKeyboardEvent(key, state);
        _eventManager->Enqueue(CreateUnique<KeyboardEvent>(key, state));
        _pressedKeys.erase(wParam);
        break;
      }
      case WM_LBUTTONDOWN:
      {
        MOUSE_BUTTON_MESSAGE(LEFT, Pressed)
      }
      case WM_LBUTTONUP:
      {
        MOUSE_BUTTON_MESSAGE(LEFT, Released)
      }
      case WM_RBUTTONDOWN:
      {
        MOUSE_BUTTON_MESSAGE(RIGHT, Pressed)
      }
      case WM_RBUTTONUP:
      {
        MOUSE_BUTTON_MESSAGE(RIGHT, Released)
      }
      case WM_MBUTTONDOWN:
      {
        MOUSE_BUTTON_MESSAGE(MIDDLE, Pressed)
      }
      case WM_MBUTTONUP:
      {
        MOUSE_BUTTON_MESSAGE(MIDDLE, Released)
      }
      case WM_XBUTTONDOWN:
      {
        const auto button =
          GET_XBUTTON_WPARAM(wParam) & XBUTTON1 ? MouseButton::THUMB_1 : MouseButton::THUMB_2;
        const auto state = MouseButtonState::Pressed;

        OnMouseButtonEvent(button, state);
        _eventManager->Enqueue(CreateUnique<MouseButtonEvent>(button, state));
        break;
      }
      case WM_XBUTTONUP:
      {
        const auto button =
          GET_XBUTTON_WPARAM(wParam) & XBUTTON1 ? MouseButton::THUMB_1 : MouseButton::THUMB_2;
        const auto state = MouseButtonState::Released;

        OnMouseButtonEvent(button, state);
        _eventManager->Enqueue(CreateUnique<MouseButtonEvent>(button, state));
        break;
      }
      case WM_MOUSEWHEEL:
      {
        const auto delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
        OnScrollWheelEvent(delta);
        _eventManager->Enqueue(CreateUnique<ScrollWheelEvent>(delta));
        break;
      }

      case WM_INPUT:
      {
        UINT dwSize = sizeof(RAWINPUT);
        BYTE lpb[sizeof(RAWINPUT)] {};

        ::GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, lpb, &dwSize,
                          sizeof(RAWINPUTHEADER));

        RAWINPUT raw {};
        std::memcpy(&raw, lpb, dwSize);

        if (raw.header.dwType == RIM_TYPEMOUSE)
        {
          // get mouse delta from raw input data
          float x = static_cast<float>(raw.data.mouse.lLastX);
          float y = static_cast<float>(raw.data.mouse.lLastY);
          float clientX = 0;
          float clientY = 0;

          // get mouse position in client space
          POINT point {};
          if (::GetCursorPos(&point))
          {
            ::ScreenToClient(windowHandle, &point);
            clientX = static_cast<float>(point.x);
            clientY = static_cast<float>(point.y);
          }

          OnMouseMoveEvent(x, y, clientX, clientY);
          _eventManager->Enqueue(CreateUnique<MouseMoveEvent>(x, y, clientX, clientY));
        }
        else if ((raw.data.mouse.usButtonFlags & RI_MOUSE_WHEEL) == RI_MOUSE_WHEEL)
        {
          const auto delta = static_cast<float>(static_cast<uint16>(raw.data.mouse.usButtonData));
          OnScrollWheelEvent(delta);
          _eventManager->Enqueue(CreateUnique<ScrollWheelEvent>(delta));
        }
        break;
      }
      default: return false;
    }

    return true;

#undef MOUSE_BUTTON_MESSAGE
  }

  Key Win32Input::KeyCodeToEngineKey(const WPARAM keyCode) const noexcept
  {
    switch (keyCode)
    {
      case 0x30:           return Key::NUM_0;
      case 0x31:           return Key::NUM_1;
      case 0x32:           return Key::NUM_2;
      case 0x33:           return Key::NUM_3;
      case 0x34:           return Key::NUM_4;
      case 0x35:           return Key::NUM_5;
      case 0x36:           return Key::NUM_6;
      case 0x37:           return Key::NUM_7;
      case 0x38:           return Key::NUM_8;
      case 0x39:           return Key::NUM_9;
      case 0x41:           return Key::A;
      case 0x42:           return Key::B;
      case 0x43:           return Key::C;
      case 0x44:           return Key::D;
      case 0x45:           return Key::E;
      case 0x46:           return Key::F;
      case 0x47:           return Key::G;
      case 0x48:           return Key::H;
      case 0x49:           return Key::I;
      case 0x4a:           return Key::J;
      case 0x4b:           return Key::K;
      case 0x4c:           return Key::L;
      case 0x4d:           return Key::M;
      case 0x4e:           return Key::N;
      case 0x4f:           return Key::O;
      case 0x50:           return Key::P;
      case 0x51:           return Key::Q;
      case 0x52:           return Key::R;
      case 0x53:           return Key::S;
      case 0x54:           return Key::T;
      case 0x55:           return Key::U;
      case 0x56:           return Key::V;
      case 0x57:           return Key::W;
      case 0x58:           return Key::X;
      case 0x59:           return Key::Y;
      case 0x5a:           return Key::Z;
      case VK_TAB:         return Key::TAB;
      case VK_SPACE:       return Key::SPACE;
      case VK_ESCAPE:      return Key::ESCAPE;
      case VK_LSHIFT:      return Key::SHIFT;
      case VK_RSHIFT:      return Key::RIGHT_SHIFT;
      case VK_F17:         return Key::F17;
      case VK_DECIMAL:     return Key::KEYPAD_DECIMAL;
      case VK_MULTIPLY:    return Key::KEYPAD_MULTIPLY;
      case VK_OEM_PLUS:    return Key::KEYPAD_PLUS;
      case VK_VOLUME_UP:   return Key::VOLUME_UP;
      case VK_VOLUME_DOWN: return Key::VOLUME_DOWN;
      case VK_VOLUME_MUTE: return Key::MUTE;
      case VK_DIVIDE:      return Key::KEYPAD_DIVIDE;
      case VK_OEM_MINUS:   return Key::KEYPAD_MINUS;
      case VK_F18:         return Key::F18;
      case VK_F19:         return Key::F19;
      case VK_NUMPAD0:     return Key::KEYPAD_0;
      case VK_NUMPAD1:     return Key::KEYPAD_1;
      case VK_NUMPAD2:     return Key::KEYPAD_2;
      case VK_NUMPAD3:     return Key::KEYPAD_3;
      case VK_NUMPAD4:     return Key::KEYPAD_4;
      case VK_NUMPAD5:     return Key::KEYPAD_5;
      case VK_NUMPAD6:     return Key::KEYPAD_6;
      case VK_NUMPAD7:     return Key::KEYPAD_7;
      case VK_F20:         return Key::F20;
      case VK_NUMPAD8:     return Key::KEYPAD_8;
      case VK_NUMPAD9:     return Key::KEYPAD_9;
      case VK_F5:          return Key::F5;
      case VK_F6:          return Key::F6;
      case VK_F7:          return Key::F7;
      case VK_F3:          return Key::F3;
      case VK_F8:          return Key::F8;
      case VK_F9:          return Key::F9;
      case VK_F11:         return Key::F11;
      case VK_F13:         return Key::F13;
      case VK_F16:         return Key::F16;
      case VK_F14:         return Key::F14;
      case VK_F10:         return Key::F10;
      case VK_F12:         return Key::F12;
      case VK_F15:         return Key::F15;
      case VK_HELP:        return Key::HELP;
      case VK_HOME:        return Key::HOME;
      case VK_F4:          return Key::F4;
      case VK_END:         return Key::END;
      case VK_F2:          return Key::F2;
      case VK_F1:          return Key::F1;
      case VK_LEFT:        return Key::LEFT_ARROW;
      case VK_RIGHT:       return Key::RIGHT_ARROW;
      case VK_DOWN:        return Key::DOWN_ARROW;
      case VK_UP:          return Key::UP_ARROW;
      case VK_RETURN:      return Key::ENTER;
      default:             return Key::UNKNOWN;
    }
  }
}