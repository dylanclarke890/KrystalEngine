#include "Krystal.PAL/Win32/Win32Window.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.PAL/IWindow.hpp"
#include "Krystal.PAL/Platform.hpp"
#include "Krystal.PAL/Win32/Utils.hpp"
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
    using namespace krys::pal::win32;

    Win32Window *window {nullptr};
    if (message == WM_NCCREATE)
    {
      auto pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
      window = static_cast<Win32Window *>(pCreate->lpCreateParams);
      ::SetWindowLongPtrA(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }
    else
    {
      window = reinterpret_cast<Win32Window *>(::GetWindowLongPtrA(handle, GWLP_USERDATA));
    }

    if (window)
    {
      return window->HandleMessage(handle, message, wParam, lParam);
    }

    return ::DefWindowProcW(handle, message, wParam, lParam);
  }

  /// @brief The engine key for a key message's virtual key code and `lParam` (its scan code and extended
  /// bit, which tell the left and right modifiers and the keypad's Enter apart).
  // FIXME: Insert, Print Screen, Pause, Scroll Lock, Num Lock and the context menu key, once the engine has
  // keys for them.
  static krys::pal::Key KeyCodeToEngineKey(const WPARAM keyCode, const LPARAM lParam) noexcept
  {
    using krys::pal::Key;
    const bool extended = (HIWORD(lParam) & KF_EXTENDED) != 0;
    switch (keyCode)
    {
      case 0x30:          return Key::Num0;
      case 0x31:          return Key::Num1;
      case 0x32:          return Key::Num2;
      case 0x33:          return Key::Num3;
      case 0x34:          return Key::Num4;
      case 0x35:          return Key::Num5;
      case 0x36:          return Key::Num6;
      case 0x37:          return Key::Num7;
      case 0x38:          return Key::Num8;
      case 0x39:          return Key::Num9;
      case 0x41:          return Key::A;
      case 0x42:          return Key::B;
      case 0x43:          return Key::C;
      case 0x44:          return Key::D;
      case 0x45:          return Key::E;
      case 0x46:          return Key::F;
      case 0x47:          return Key::G;
      case 0x48:          return Key::H;
      case 0x49:          return Key::I;
      case 0x4a:          return Key::J;
      case 0x4b:          return Key::K;
      case 0x4c:          return Key::L;
      case 0x4d:          return Key::M;
      case 0x4e:          return Key::N;
      case 0x4f:          return Key::O;
      case 0x50:          return Key::P;
      case 0x51:          return Key::Q;
      case 0x52:          return Key::R;
      case 0x53:          return Key::S;
      case 0x54:          return Key::T;
      case 0x55:          return Key::U;
      case 0x56:          return Key::V;
      case 0x57:          return Key::W;
      case 0x58:          return Key::X;
      case 0x59:          return Key::Y;
      case 0x5a:          return Key::Z;
      case VK_OEM_PLUS:   return Key::Equal;
      case VK_OEM_MINUS:  return Key::Minus;
      case VK_OEM_6:      return Key::RightBracket;
      case VK_OEM_4:      return Key::LeftBracket;
      case VK_OEM_7:      return Key::Quote;
      case VK_OEM_1:      return Key::SemiColon;
      case VK_OEM_5:      return Key::Backslash;
      case VK_OEM_COMMA:  return Key::Comma;
      case VK_OEM_2:      return Key::Slash;
      case VK_OEM_PERIOD: return Key::Period;
      case VK_OEM_3:      return Key::Grave;
      case VK_TAB:        return Key::Tab;
      case VK_SPACE:      return Key::Space;
      case VK_BACK:       return Key::Del;
      case VK_DELETE:     return Key::ForwardDelete;
      case VK_ESCAPE:     return Key::Escape;
      case VK_CAPITAL:    return Key::CapsLock;
      case VK_SHIFT:
        // The right Shift has its own scan code rather than the extended bit.
        return ::MapVirtualKeyW((lParam >> 16) & 0xff, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT ? Key::RightShift
                                                                                        : Key::Shift;
      case VK_LSHIFT:      return Key::Shift;
      case VK_RSHIFT:      return Key::RightShift;
      case VK_CONTROL:     return extended ? Key::RightControl : Key::Control;
      case VK_LCONTROL:    return Key::Control;
      case VK_RCONTROL:    return Key::RightControl;
      case VK_MENU:        return extended ? Key::RightOption : Key::Option;
      case VK_LMENU:       return Key::Option;
      case VK_RMENU:       return Key::RightOption;
      case VK_LWIN:
      case VK_RWIN:        return Key::Command;
      case VK_VOLUME_UP:   return Key::VolumeUp;
      case VK_VOLUME_DOWN: return Key::VolumeDown;
      case VK_VOLUME_MUTE: return Key::Mute;
      case VK_MULTIPLY:    return Key::KeypadMultiply;
      case VK_DIVIDE:      return Key::KeypadDivide;
      case VK_SUBTRACT:    return Key::KeypadMinus;
      case VK_ADD:         return Key::KeypadPlus;
      case VK_DECIMAL:     return Key::KeypadDecimal;
      case VK_CLEAR:       return Key::KeypadClear;
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
      case VK_PRIOR:       return Key::PageUp;
      case VK_NEXT:        return Key::PageDown;
      case VK_LEFT:        return Key::LeftArrow;
      case VK_RIGHT:       return Key::RightArrow;
      case VK_DOWN:        return Key::DownArrow;
      case VK_UP:          return Key::UpArrow;
      case VK_RETURN:      return extended ? Key::KeypadEnter : Key::Enter;
      default:             return Key::UNKNOWN;
    }
  }

  /// @brief The modifier keys held as the current message was sent.
  static krys::pal::KeyModifiers CurrentModifiers() noexcept
  {
    using krys::pal::KeyModifier;
    krys::pal::KeyModifiers modifiers;
    auto isDown = [](int key)
    {
      return (::GetKeyState(key) & 0x8000) != 0;
    };
    if (isDown(VK_SHIFT))
    {
      modifiers.Add(KeyModifier::Shift);
    }
    if (isDown(VK_CONTROL))
    {
      modifiers.Add(KeyModifier::Control);
    }
    if (isDown(VK_MENU))
    {
      modifiers.Add(KeyModifier::Alt);
    }
    if (isDown(VK_LWIN) || isDown(VK_RWIN))
    {
      modifiers.Add(KeyModifier::Meta);
    }
    if ((::GetKeyState(VK_CAPITAL) & 1) != 0)
    {
      modifiers.Add(KeyModifier::CapsLock);
    }
    return modifiers;
  }

  /// @brief The characters @p keyCode types with the keyboard @p state, without control characters except the
  /// carriage return, and without changing the layout's dead key state.
  static krys::utf8_string TypedText(const WPARAM keyCode, const LPARAM lParam, const BYTE *state) noexcept
  {
    // Don't change the keyboard state (Windows 10 1607 and later), so dead keys still combine with the next
    // key in the messages the system translates.
    constexpr UINT DontChangeKeyboardState = 1u << 2;
    wchar_t buffer[8] {};
    const int count =
      ::ToUnicodeEx(static_cast<UINT>(keyCode), (lParam >> 16) & 0xff, state, buffer,
                    static_cast<int>(std::size(buffer)), DontChangeKeyboardState, ::GetKeyboardLayout(0));
    if (count <= 0)
    {
      return {};
    }

    std::wstring text;
    for (int i = 0; i < count; ++i)
    {
      if (buffer[i] >= 0x20 && buffer[i] != 0x7f || buffer[i] == L'\r')
      {
        text.push_back(buffer[i]);
      }
    }
    if (text.empty())
    {
      return {};
    }
    const int size =
      ::WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), NULL, 0, NULL, NULL);
    krys::utf8_string result(static_cast<size_t>(size), u8'\0');
    ::WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                          reinterpret_cast<char *>(result.data()), size, NULL, NULL);
    return result;
  }

  /// @brief The text a key press types, with the modifiers held and with only Shift and Caps Lock.
  static krys::pal::KeyText KeyTextFor(const WPARAM keyCode, const LPARAM lParam) noexcept
  {
    BYTE state[256] {};
    if (!::GetKeyboardState(state))
    {
      return {};
    }
    krys::pal::KeyText text;
    text.Text = TypedText(keyCode, lParam, state);
    for (int key : {VK_CONTROL, VK_LCONTROL, VK_RCONTROL, VK_MENU, VK_LMENU, VK_RMENU})
    {
      state[key] = 0;
    }
    text.UnmodifiedText = TypedText(keyCode, lParam, state);
    return text;
  }

  static LPCWSTR CursorShapeToResource(const krys::pal::CursorShape cursor) noexcept
  {
    using krys::pal::CursorShape;
    switch (cursor)
    {
      case CursorShape::Arrow:      return IDC_ARROW;
      case CursorShape::IBeam:      return IDC_IBEAM;
      case CursorShape::Hand:       return IDC_HAND;
      case CursorShape::Crosshair:  return IDC_CROSS;
      case CursorShape::Wait:       return IDC_WAIT;
      case CursorShape::Progress:   return IDC_APPSTARTING;
      case CursorShape::Help:       return IDC_HELP;
      case CursorShape::NotAllowed: return IDC_NO;
      case CursorShape::Move:       return IDC_SIZEALL;
      case CursorShape::ResizeNS:   return IDC_SIZENS;
      case CursorShape::ResizeEW:   return IDC_SIZEWE;
      case CursorShape::ResizeNESW: return IDC_SIZENESW;
      case CursorShape::ResizeNWSE: return IDC_SIZENWSE;
      case CursorShape::Hidden:     return NULL;
    }
    return IDC_ARROW;
  }

  static std::wstring ToWideString(const krys::string &str)
  {
    const int sizeNeeded =
      ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
    std::wstring wideStr(sizeNeeded, 0);
    ::MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), wideStr.data(), sizeNeeded);
    return wideStr;
  }
}

namespace krys::pal
{
  Expected<UniquePtr<IWindow>> CreateWindow(const WindowSettings &settings) noexcept
  {
    try
    {
      return Expected<UniquePtr<IWindow>>(CreateUnique<win32::Win32Window>(settings));
    }
    catch (const std::exception &e)
    {
      return Unexpected(e.what());
    }
  }

  namespace win32
  {
    Win32Window::Win32Window(const WindowSettings &settings)
        : _settings(settings), _callbacks(), _handle(nullptr)
    {
      ::SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

      const auto instance = ::GetModuleHandleW(NULL);
      if (!instance)
      {
        throw std::runtime_error("Failed to get module handle: " + win32::GetLastErrorAsString());
      }

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
      _class = WNDCLASSW {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = &WindowProc,
        .cbWndExtra = sizeof(Win32Window *),
        .hInstance = instance,
        .hCursor = ::LoadCursorW(instance, IDC_ARROW),
        .hbrBackground = NULL,
        .lpszMenuName = NULL,
        .lpszClassName = L"KrystalWindowClass",
      };

      auto result = ::RegisterClassW(&_class);
      if (!result && ::GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
      {
        throw std::runtime_error("Failed to register window class: " + win32::GetLastErrorAsString());
      }
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
        throw std::runtime_error("Failed to adjust window rect: " + win32::GetLastErrorAsString());

      int width = dimensions.right - dimensions.left;
      int height = dimensions.bottom - dimensions.top;

      _handle = ::CreateWindowExW(0, _class.lpszClassName, ToWideString(_settings.Title).c_str(), styles,
                                  CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, instance, this);
      if (!_handle)
        throw std::runtime_error("Failed to create window: " + win32::GetLastErrorAsString());
    }

    void Win32Window::RegisterRawInput() const
    {
      // TODO(fix): account for dpi?
      RAWINPUTDEVICE rid {};
      rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
      rid.usUsage = HID_USAGE_GENERIC_MOUSE;
      rid.dwFlags = RIDEV_INPUTSINK;
      rid.hwndTarget = _handle;

      auto result = ::RegisterRawInputDevices(&rid, 1, sizeof(rid));
      if (!result)
        throw std::runtime_error("Failed to register raw input device: " + win32::GetLastErrorAsString());
    }

    void Win32Window::ProcessMessages() noexcept
    {
      MSG message {};
      while (::PeekMessageW(&message, _handle, 0, 0, PM_REMOVE) != 0)
      {
        ::TranslateMessage(&message);
        ::DispatchMessageW(&message);
      }
    }

    LRESULT Win32Window::HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam) const noexcept
    {
      static HashSet<WPARAM> _pressedKeys;
      const auto handle = GetWindowHandle();
      switch (message)
      {
        case WM_SETCURSOR:
        {
          if (LOWORD(lParam) == HTCLIENT)
          {
            ApplyCursor();
            return TRUE;
          }
          break;
        }
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
        {
          if (_callbacks.OnFocusChange)
          {
            _callbacks.OnFocusChange(handle, message == WM_SETFOCUS);
          }
          return 0;
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
#pragma region Input Messages
        // Alt combinations (and F10) come as system keys; they still go on to DefWindowProc, for Alt+F4.
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
          if (_callbacks.OnKey)
          {
            const auto key = KeyCodeToEngineKey(wParam, lParam);
            const auto state = _pressedKeys.contains(wParam) ? KeyState::Held : KeyState::Pressed;
            _callbacks.OnKey(handle, key, state, CurrentModifiers(), KeyTextFor(wParam, lParam));
            _pressedKeys.emplace(wParam);
          }
          break;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
          if (_callbacks.OnKey)
          {
            const auto key = KeyCodeToEngineKey(wParam, lParam);
            const auto state = KeyState::Released;
            _callbacks.OnKey(handle, key, state, CurrentModifiers(), {});
            _pressedKeys.erase(wParam);
          }
          break;
        }
        // The character of an Alt combination: DefWindowProc would look for a menu mnemonic and beep when
        // there isn't one, so only Alt+Space (the window menu) goes on to it.
        case WM_SYSCHAR:
        {
          if (wParam == L' ')
          {
            break;
          }
          return 0;
        }
        // Alt or F10 alone would enter menu mode, which a window without a menu has no use for, and which
        // eats the next key. Mnemonics (lParam is the character) still go on, for Alt+Space.
        case WM_SYSCOMMAND:
        {
          if ((wParam & 0xfff0) == SC_KEYMENU && lParam == 0)
          {
            return 0;
          }
          break;
        }

#define BUTTON_STATE(BTN, STATE)                                                                             \
  const auto button = MouseButton::BTN;                                                                      \
  const auto state = MouseButtonState::STATE;

        case WM_LBUTTONDOWN:
        {
          if (_callbacks.OnMouseButton)
          {
            BUTTON_STATE(Left, Pressed)
            _callbacks.OnMouseButton(handle, button, state, CurrentModifiers());
          }
          break;
        }
        case WM_LBUTTONUP:
        {
          if (_callbacks.OnMouseButton)
          {
            BUTTON_STATE(Left, Released)
            _callbacks.OnMouseButton(handle, button, state, CurrentModifiers());
          }
          break;
        }
        case WM_RBUTTONDOWN:
        {
          if (_callbacks.OnMouseButton)
          {
            BUTTON_STATE(Right, Pressed)
            _callbacks.OnMouseButton(handle, button, state, CurrentModifiers());
          }
          break;
        }
        case WM_RBUTTONUP:
        {
          if (_callbacks.OnMouseButton)
          {
            BUTTON_STATE(Right, Released)
            _callbacks.OnMouseButton(handle, button, state, CurrentModifiers());
          }
          break;
        }
        case WM_MBUTTONDOWN:
        {
          if (_callbacks.OnMouseButton)
          {
            BUTTON_STATE(Middle, Pressed)
            _callbacks.OnMouseButton(handle, button, state, CurrentModifiers());
          }
          break;
        }
        case WM_MBUTTONUP:
        {
          if (_callbacks.OnMouseButton)
          {
            BUTTON_STATE(Middle, Released)
            _callbacks.OnMouseButton(handle, button, state, CurrentModifiers());
          }
          break;
        }

#undef BUTTON_STATE

        case WM_XBUTTONDOWN:
        {
          if (_callbacks.OnMouseButton)
          {
            const auto button =
              GET_XBUTTON_WPARAM(wParam) & XBUTTON1 ? MouseButton::Thumb1 : MouseButton::Thumb2;
            const auto state = MouseButtonState::Pressed;

            _callbacks.OnMouseButton(handle, button, state, CurrentModifiers());
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

            _callbacks.OnMouseButton(handle, button, state, CurrentModifiers());
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

            _callbacks.OnMouseMove(handle, x, y, clientX, clientY, CurrentModifiers());
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
#pragma endregion
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
          if (::IsZoomed(_handle))
          {
            break; // let default proc handle maximized window
          }

          MINMAXINFO *info = reinterpret_cast<MINMAXINFO *>(lParam);

          UINT dpi = GetDpiForWindow(window);
          float scale = dpi / 96.0f;

          // Scale bounds to match current DPI
          info->ptMinTrackSize.x = static_cast<LONG>(_settings.SizeBounds.Min.Width * scale);
          info->ptMinTrackSize.y = static_cast<LONG>(_settings.SizeBounds.Min.Height * scale);
          info->ptMaxTrackSize.x = static_cast<LONG>(_settings.SizeBounds.Max.Width * scale);
          info->ptMaxTrackSize.y = static_cast<LONG>(_settings.SizeBounds.Max.Height * scale);

          return 0;
        }

        case WM_DPICHANGED:
        {
          RECT *rect = reinterpret_cast<RECT *>(lParam);
          ::SetWindowPos(window, NULL, rect->left, rect->top, rect->right - rect->left,
                         rect->bottom - rect->top, SWP_NOZORDER);
          if (_callbacks.OnDPIChange)
          {
            assert(HIWORD(wParam) == LOWORD(wParam) && "Unexpected dpi");
            _callbacks.OnDPIChange(handle, HIWORD(wParam));
          }
          return 0;
        }
      }

      return ::DefWindowProcW(window, message, wParam, lParam);
    }

    void Win32Window::SetTitle(const string &title) noexcept
    {
      auto result = ::SetWindowTextW(_handle, ToWideString(title).c_str());
      assert(result);
      if (result)
      {
        _settings.Title = title;
      }
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

    void Win32Window::SetCursor(CursorShape cursor) noexcept
    {
      if (_cursor == cursor)
      {
        return;
      }
      _cursor = cursor;

      // WM_SETCURSOR only comes as the mouse moves, so a change under a still mouse is applied now.
      POINT point {};
      if (!::GetCursorPos(&point) || ::WindowFromPoint(point) != _handle)
      {
        return;
      }
      RECT client {};
      if (::ScreenToClient(_handle, &point) && ::GetClientRect(_handle, &client)
          && ::PtInRect(&client, point))
      {
        ApplyCursor();
      }
    }

    CursorShape Win32Window::GetCursor() const noexcept
    {
      return _cursor;
    }

    void Win32Window::ApplyCursor() const noexcept
    {
      if (_cursor == CursorShape::Hidden)
      {
        ::SetCursor(NULL);
        return;
      }
      ::SetCursor(::LoadCursorW(NULL, CursorShapeToResource(_cursor)));
    }
  }
}