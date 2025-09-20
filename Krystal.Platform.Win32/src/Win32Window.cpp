#include "Krystal.Platform.Win32/Win32Window.hpp"
#include "Krystal.Lib/Set.hpp"
#include "Krystal.Platform.Win32/Utils.hpp"
#include "Krystal.Platform/IWindow.hpp"
#include "Krystal.Platform/Platform.hpp"
#include <cassert>
#include <hidusage.h>
#include <string>
#include <windows.h>

#ifdef CreateWindow
  #undef CreateWindow
#endif

namespace
{
  static LRESULT CALLBACK WindowProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) noexcept
  {
    using namespace Krys::Platform::Win32;

    Win32Window *window = nullptr;
    if (message == WM_NCCREATE)
    {
      CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
      window = static_cast<Win32Window *>(pCreate->lpCreateParams);
      ::SetWindowLongPtrA(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }
    else
    {
      window = reinterpret_cast<Win32Window *>(::GetWindowLongPtrA(handle, GWLP_USERDATA));
    }

    if (window)
      return window->HandleMessage(handle, message, wParam, lParam);
    return ::DefWindowProc(handle, message, wParam, lParam);
  }

  static Krys::Platform::Key KeyCodeToEngineKey(const WPARAM keyCode) noexcept
  {
    using Krys::Platform::Key;
    switch (keyCode)
    {
      case 0x30:           return Key::Num0;
      case 0x31:           return Key::Num1;
      case 0x32:           return Key::Num2;
      case 0x33:           return Key::Num3;
      case 0x34:           return Key::Num4;
      case 0x35:           return Key::Num5;
      case 0x36:           return Key::Num6;
      case 0x37:           return Key::Num7;
      case 0x38:           return Key::Num8;
      case 0x39:           return Key::Num9;
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
      case VK_TAB:         return Key::Tab;
      case VK_SPACE:       return Key::Space;
      case VK_ESCAPE:      return Key::Escape;
      case VK_LSHIFT:      return Key::Shift;
      case VK_RSHIFT:      return Key::RightShift;
      case VK_VOLUME_UP:   return Key::VolumeUp;
      case VK_VOLUME_DOWN: return Key::VolumeDown;
      case VK_VOLUME_MUTE: return Key::Mute;
      case VK_MULTIPLY:    return Key::KeypadMultiply;
      case VK_DIVIDE:      return Key::KeypadDivide;
      case VK_OEM_MINUS:   return Key::KeypadMinus;
      case VK_OEM_PLUS:    return Key::KeypadPlus;
      case VK_DECIMAL:     return Key::KeypadDecimal;
      case VK_NUMPAD0:     return Key::Keypad0;
      case VK_NUMPAD1:     return Key::Keypad1;
      case VK_NUMPAD2:     return Key::Keypad2;
      case VK_NUMPAD3:     return Key::Keypad3;
      case VK_NUMPAD4:     return Key::Keypad4;
      case VK_NUMPAD5:     return Key::Keypad5;
      case VK_NUMPAD6:     return Key::Keypad6;
      case VK_NUMPAD7:     return Key::Keypad7;
      case VK_NUMPAD8:     return Key::Keypad8;
      case VK_NUMPAD9:     return Key::Keypad9;
      case VK_F1:          return Key::F1;
      case VK_F2:          return Key::F2;
      case VK_F3:          return Key::F3;
      case VK_F4:          return Key::F4;
      case VK_F5:          return Key::F5;
      case VK_F6:          return Key::F6;
      case VK_F7:          return Key::F7;
      case VK_F8:          return Key::F8;
      case VK_F9:          return Key::F9;
      case VK_F10:         return Key::F10;
      case VK_F11:         return Key::F11;
      case VK_F12:         return Key::F12;
      case VK_F13:         return Key::F13;
      case VK_F14:         return Key::F14;
      case VK_F15:         return Key::F15;
      case VK_F16:         return Key::F16;
      case VK_F17:         return Key::F17;
      case VK_F18:         return Key::F18;
      case VK_F19:         return Key::F19;
      case VK_F20:         return Key::F20;
      case VK_HELP:        return Key::Help;
      case VK_HOME:        return Key::Home;
      case VK_END:         return Key::End;

      case VK_LEFT:        return Key::LeftArrow;
      case VK_RIGHT:       return Key::RightArrow;
      case VK_DOWN:        return Key::DownArrow;
      case VK_UP:          return Key::UpArrow;
      case VK_RETURN:      return Key::Enter;
      default:             return Key::UNKNOWN;
    }
  }
}

namespace Krys::Platform
{
  Expected<Unique<IWindow>> CreateWindow(const WindowSettings &settings) noexcept
  {
    try
    {
      return Expected<Unique<IWindow>>(CreateUnique<Win32::Win32Window>(settings));
    }
    catch (const std::exception &e)
    {
      return Unexpected(e.what());
    }
  }

  namespace Win32
  {
    Win32Window::Win32Window(const WindowSettings &settings)
        : _settings(settings), _callbacks(), _handle(nullptr)
    {
      const auto instance = ::GetModuleHandle(NULL);
      if (!instance)
        throw std::runtime_error("Failed to get module handle: " + Win32::GetLastErrorAsString());

      CreateWindowClass(instance);
      CreateWindowHandle(instance);
      RegisterRawInput();
    }

    Win32Window::~Win32Window() noexcept
    {
    }

    WindowHandle Win32Window::GetWindowHandle() const noexcept
    {
      return NativeHandle {_handle};
    }

    void Win32Window::CreateWindowClass(HMODULE instance)
    {
      _class = {};
      _class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
      _class.hInstance = instance;
      _class.lpfnWndProc = &WindowProc;
      _class.lpszMenuName = NULL;
      _class.hbrBackground = NULL;
      _class.hCursor = ::LoadCursor(instance, IDC_ARROW);
      _class.cbWndExtra = sizeof(Win32Window *);
      _class.lpszClassName = L"KrystalWindowClass";

      auto result = ::RegisterClass(&_class);
      if (!result && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        throw std::runtime_error("Failed to register window class: " + Win32::GetLastErrorAsString());
    }

    void Win32Window::CreateWindowHandle(HMODULE instance)
    {
      RECT dimensions = {0, 0, (LONG)_settings.Size.Width, (LONG)_settings.Size.Height};

      int styles = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
      if (_settings.Visible)
        styles |= WS_VISIBLE;
      if (_settings.SizeBounds.Resizable)
        styles |= WS_SIZEBOX | WS_MAXIMIZEBOX;

      auto result = ::AdjustWindowRect(&dimensions, styles, 0);
      if (!result)
        throw std::runtime_error("Failed to adjust window rect: " + Win32::GetLastErrorAsString());

      int width = dimensions.right - dimensions.left;
      int height = dimensions.bottom - dimensions.top;

      _handle = ::CreateWindowEx(0, _class.lpszClassName, ToWideString(_settings.Title).c_str(), styles,
                                 CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, instance, this);
      if (!_handle)
        throw std::runtime_error("Failed to create window: " + Win32::GetLastErrorAsString());
    }

    void Win32Window::RegisterRawInput() const
    {
      // TODO: account for dpi?
      RAWINPUTDEVICE rid {};
      rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
      rid.usUsage = HID_USAGE_GENERIC_MOUSE;
      rid.dwFlags = RIDEV_INPUTSINK;
      rid.hwndTarget = _handle;

      auto result = ::RegisterRawInputDevices(&rid, 1, sizeof(rid));
      if (!result)
        throw std::runtime_error("Failed to register raw input device: " + Win32::GetLastErrorAsString());
    }

    void Win32Window::ProcessMessages() noexcept
    {
      MSG message {};
      while (::PeekMessage(&message, _handle, 0, 0, PM_REMOVE) != 0)
      {
        ::TranslateMessage(&message);
        ::DispatchMessage(&message);
      }
    }

    LRESULT Win32Window::HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam) const noexcept
    {
      static Set<WPARAM> _pressedKeys;
      const auto handle = GetWindowHandle();
      switch (message)
      {
        case WM_SETCURSOR:
        {
          if (LOWORD(lParam) == HTCLIENT)
          {
            ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
            return TRUE;
          }
          break;
        }
        case WM_CLOSE:
        {
          if (_callbacks.OnClose)
          {
            _callbacks.OnClose(handle);
            return 0;
          }
          break;
        }
        case WM_KEYDOWN:
        {
          if (_callbacks.OnKey)
          {
            const auto key = KeyCodeToEngineKey(wParam);
            const auto state = _pressedKeys.contains(wParam) ? KeyState::Held : KeyState::Pressed;
            _callbacks.OnKey(handle, key, state);
            _pressedKeys.emplace(wParam);
          }
          break;
        }
        case WM_KEYUP:
        {
          if (_callbacks.OnKey)
          {
            const auto key = KeyCodeToEngineKey(wParam);
            const auto state = KeyState::Released;
            _callbacks.OnKey(handle, key, state);
            _pressedKeys.erase(wParam);
          }
          break;
        }

#define BTN_STATE(BTN, STATE)                                                                                \
  const auto button = MouseButton::BTN;                                                                      \
  const auto state = MouseButtonState::STATE;

        case WM_LBUTTONDOWN:
        {
          if (_callbacks.OnMouseButton)
          {
            BTN_STATE(Left, Pressed)
            _callbacks.OnMouseButton(handle, button, state);
          }
          break;
        }
        case WM_LBUTTONUP:
        {
          if (_callbacks.OnMouseButton)
          {
            BTN_STATE(Left, Released)
            _callbacks.OnMouseButton(handle, button, state);
          }
          break;
        }
        case WM_RBUTTONDOWN:
        {
          if (_callbacks.OnMouseButton)
          {
            BTN_STATE(Right, Pressed)
            _callbacks.OnMouseButton(handle, button, state);
          }
          break;
        }
        case WM_RBUTTONUP:
        {
          if (_callbacks.OnMouseButton)
          {
            BTN_STATE(Right, Released)
            _callbacks.OnMouseButton(handle, button, state);
          }
          break;
        }
        case WM_MBUTTONDOWN:
        {
          if (_callbacks.OnMouseButton)
          {
            BTN_STATE(Middle, Pressed)
            _callbacks.OnMouseButton(handle, button, state);
          }
          break;
        }
        case WM_MBUTTONUP:
        {
          if (_callbacks.OnMouseButton)
          {
            BTN_STATE(Middle, Released)
            _callbacks.OnMouseButton(handle, button, state);
          }
          break;
        }

#undef BTN_STATE

        case WM_XBUTTONDOWN:
        {
          if (_callbacks.OnMouseButton)
          {
            const auto button =
              GET_XBUTTON_WPARAM(wParam) & XBUTTON1 ? MouseButton::Thumb1 : MouseButton::Thumb2;
            const auto state = MouseButtonState::Pressed;

            _callbacks.OnMouseButton(handle, button, state);
          }
          break;
        }
        case WM_XBUTTONUP:
        {
          if (_callbacks.OnMouseButton)
          {
            const auto button =
              GET_XBUTTON_WPARAM(wParam) & XBUTTON1 ? MouseButton::Thumb1 : MouseButton::Thumb2;
            const auto state = MouseButtonState::Released;

            _callbacks.OnMouseButton(handle, button, state);
          }
          break;
        }
        case WM_MOUSEWHEEL:
        {
          if (_callbacks.OnMouseScroll)
          {
            const auto delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
            _callbacks.OnMouseScroll(handle, delta);
          }
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
            if (!_callbacks.OnMouseMove)
              return 0; // no mouse move callback registered, ignore
            // get mouse delta from raw input data
            float x = static_cast<float>(raw.data.mouse.lLastX);
            float y = static_cast<float>(raw.data.mouse.lLastY);
            float clientX = 0;
            float clientY = 0;

            // get mouse position in client space
            POINT point {};
            if (::GetCursorPos(&point))
            {
              ::ScreenToClient(window, &point);
              clientX = static_cast<float>(point.x);
              clientY = static_cast<float>(point.y);
            }

            _callbacks.OnMouseMove(handle, x, y, clientX, clientY);
          }
          else if ((raw.data.mouse.usButtonFlags & RI_MOUSE_WHEEL) == RI_MOUSE_WHEEL)
          {
            if (!_callbacks.OnMouseScroll)
              return 0; // no mouse scroll callback registered, ignore
            const auto delta = static_cast<float>(static_cast<uint16>(raw.data.mouse.usButtonData));
            _callbacks.OnMouseScroll(handle, delta);
          }
          break;
        }
        case WM_SIZE:
        {
          static bool isMinimised = false;
          switch (wParam)
          {
            case SIZE_MINIMIZED:
              isMinimised = true;
              if (_callbacks.OnMinimise)
              {
                _callbacks.OnMinimise(handle);
              }
              break;
            case SIZE_MAXIMIZED:
            case SIZE_RESTORED:
              if (isMinimised)
              {
                isMinimised = false;
                if (_callbacks.OnRestore)
                {
                  _callbacks.OnRestore(handle);
                }
              }

              if (_callbacks.OnResize)
              {
                const auto width = LOWORD(lParam);
                const auto height = HIWORD(lParam);
                _callbacks.OnResize(handle, width, height);
              }
              break;
          }
          break;
        }
        case WM_GETMINMAXINFO:
        {
          MINMAXINFO &info = *reinterpret_cast<MINMAXINFO *>(lParam);
          info.ptMaxTrackSize.x = _settings.SizeBounds.Max.Width;
          info.ptMaxTrackSize.y = _settings.SizeBounds.Max.Height;
          info.ptMinTrackSize.x = _settings.SizeBounds.Min.Width;
          info.ptMinTrackSize.y = _settings.SizeBounds.Min.Height;
        }
      }

      return ::DefWindowProc(window, message, wParam, lParam);
    }

    void Win32Window::SetTitle(const string &title) noexcept
    {
      auto result = ::SetWindowText(_handle, ToWideString(title).c_str());
      assert(result);
      if (result)
        _settings.Title = title;
    }

    const string &Win32Window::GetTitle() const noexcept
    {
      return _settings.Title;
    }

    WindowSize Win32Window::GetSize() const noexcept
    {
      RECT rect {};
      if (::GetClientRect(_handle, &rect))
      {
        return WindowSize {static_cast<uint32>(rect.right - rect.left),
                           static_cast<uint32>(rect.bottom - rect.top)};
      }
      return WindowSize {0, 0};
    }

    void Win32Window::Show() noexcept
    {
      auto result = ::ShowWindow(_handle, SW_SHOW);
      assert(result);
    }

    void Win32Window::Hide() noexcept
    {
      auto result = ::ShowWindow(_handle, SW_HIDE);
      assert(result);
    }

    void Win32Window::SetCallbacks(WindowCallbacks &&callbacks) noexcept
    {
      _callbacks = std::move(callbacks);
    }
  }
}