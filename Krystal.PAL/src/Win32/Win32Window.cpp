#include "Krystal.PAL/Win32/Win32Window.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.PAL/IWindow.hpp"
#include "Krystal.PAL/Platform.hpp"
#include "Krystal.PAL/Win32/Utils.hpp"
#include <algorithm>
#include <cassert>
#include <hidusage.h>
#include <imm.h>
#include <string>
#include <windows.h>
#include <windowsx.h>

#ifndef WM_TABLET_QUERYSYSTEMGESTURESTATUS
  #define WM_TABLET_QUERYSYSTEMGESTURESTATUS 0x02CC
#endif

#ifdef CreateWindow
  #undef CreateWindow
#endif

#pragma comment(lib, "Imm32.lib")

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

  // The answers to WM_TABLET_QUERYSYSTEMGESTURESTATUS (tpcshrd.h).
  constexpr LRESULT TabletDisablePressAndHold = 0x00'00'00'01;
  constexpr LRESULT TabletDisableFlicks = 0x00'01'00'00;

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

  /// @brief Whether @p character is text: not a control character, except the carriage return.
  static bool IsTypedCharacter(const wchar_t character) noexcept
  {
    return (character >= 0x20 && character != 0x7f) || character == L'\r';
  }

  /// @brief @p text in UTF-8 (a lone surrogate as the replacement character).
  static krys::utf8_string ToUtf8(const std::wstring_view text) noexcept
  {
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

  /// @brief Each of @p text's UTF-16 offsets (and its end) as the offset in its UTF-8 form, as `ToUtf8()`
  /// makes it: the second half of a surrogate pair maps to its first's.
  static krys::List<krys::uint32> Utf8Offsets(const std::wstring_view text) noexcept
  {
    krys::List<krys::uint32> offsets(text.size() + 1, 0);
    krys::uint32 offset = 0;
    for (size_t i = 0; i < text.size(); ++i)
    {
      offsets[i] = offset;
      const wchar_t character = text[i];
      if (IS_HIGH_SURROGATE(character) && i + 1 < text.size() && IS_LOW_SURROGATE(text[i + 1]))
      {
        offsets[++i] = offset;
        offset += 4;
      }
      else
      {
        // A lone surrogate becomes the replacement character, three bytes.
        offset += character < 0x80 ? 1 : character < 0x800 ? 2 : 3;
      }
    }
    offsets[text.size()] = offset;
    return offsets;
  }

  /// @brief The virtual key code of a key message, the key's own for one the input method took
  /// (`VK_PROCESSKEY`), from its scan code.
  static WPARAM KeyCodeFor(const WPARAM keyCode, const LPARAM lParam) noexcept
  {
    if (keyCode != VK_PROCESSKEY)
    {
      return keyCode;
    }
    UINT scanCode = (lParam >> 16) & 0xff;
    if ((HIWORD(lParam) & KF_EXTENDED) != 0)
    {
      scanCode |= 0xe000;
    }
    const UINT mapped = ::MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK);
    return mapped != 0 ? mapped : keyCode;
  }

  /// @brief The window's input method context, while it's in scope.
  class InputContext
  {
    KRYS_NON_COPY_MOVABLE(InputContext);

  private:
    HWND _window;
    HIMC _context;

  public:
    explicit InputContext(HWND window) noexcept : _window(window), _context(::ImmGetContext(window))
    {
    }

    ~InputContext() noexcept
    {
      if (_context)
      {
        ::ImmReleaseContext(_window, _context);
      }
    }

    KRYS_NODISCARD HIMC Get() const noexcept
    {
      return _context;
    }

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return _context != NULL;
    }
  };

  /// @brief Part of the input method's composition, as @p index (a `GCS_*` flag) names it.
  template <typename T>
  static krys::List<T> CompositionData(HIMC context, const DWORD index) noexcept
  {
    const LONG size = ::ImmGetCompositionStringW(context, index, nullptr, 0);
    if (size <= 0)
    {
      return {};
    }
    krys::List<T> data(static_cast<size_t>(size) / sizeof(T));
    ::ImmGetCompositionStringW(context, index, data.data(), static_cast<DWORD>(data.size() * sizeof(T)));
    return data;
  }

  /// @brief The input method's composition string or result, as @p index (a `GCS_*` flag) names it.
  static std::wstring CompositionString(HIMC context, const DWORD index) noexcept
  {
    auto data = CompositionData<wchar_t>(context, index);
    return std::wstring(data.begin(), data.end());
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
      if (IsTypedCharacter(buffer[i]))
      {
        text.push_back(buffer[i]);
      }
    }
    return ToUtf8(text);
  }

  /// @brief The text a key press would type with only Shift and Caps Lock.
  static krys::utf8_string UnmodifiedTextFor(const WPARAM keyCode, const LPARAM lParam) noexcept
  {
    BYTE state[256] {};
    if (!::GetKeyboardState(state))
    {
      return {};
    }
    for (int key : {VK_CONTROL, VK_LCONTROL, VK_RCONTROL, VK_MENU, VK_LMENU, VK_RMENU})
    {
      state[key] = 0;
    }
    return TypedText(keyCode, lParam, state);
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
      case CursorShape::DragNone:
      case CursorShape::DragMove:
      case CursorShape::DragCopy:
      case CursorShape::DragLink:
      case CursorShape::Hidden:     return NULL;
    }
    return IDC_ARROW;
  }

  /// @brief The drag and drop cursor OLE shows for @p cursor (ole32.dll's cursors 1 to 4), or null if it
  /// isn't one.
  static HCURSOR DragCursor(const krys::pal::CursorShape cursor) noexcept
  {
    using krys::pal::CursorShape;
    int resource = 0;
    switch (cursor)
    {
      case CursorShape::DragNone: resource = 1; break;
      case CursorShape::DragMove: resource = 2; break;
      case CursorShape::DragCopy: resource = 3; break;
      case CursorShape::DragLink: resource = 4; break;
      default:                    return NULL;
    }

    static HMODULE ole32 = ::LoadLibraryExW(L"ole32.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (!ole32)
    {
      return NULL;
    }
    return ::LoadCursorW(ole32, MAKEINTRESOURCEW(resource));
  }

  /// @brief Captures the mouse while a button is held, so the release reaches the window wherever the mouse
  /// is (a drag in the page follows it out of the window), and releases it with the last button.
  static void UpdateMouseCapture(HWND window, UINT message, WPARAM wParam) noexcept
  {
    switch (message)
    {
      case WM_LBUTTONDOWN:
      case WM_RBUTTONDOWN:
      case WM_MBUTTONDOWN:
      case WM_XBUTTONDOWN: ::SetCapture(window); break;
      case WM_LBUTTONUP:
      case WM_RBUTTONUP:
      case WM_MBUTTONUP:
      case WM_XBUTTONUP:
      {
        constexpr WPARAM buttons = MK_LBUTTON | MK_RBUTTON | MK_MBUTTON | MK_XBUTTON1 | MK_XBUTTON2;
        if (!(GET_KEYSTATE_WPARAM(wParam) & buttons) && ::GetCapture() == window)
        {
          ::ReleaseCapture();
        }
        break;
      }
      default: break;
    }
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
      // Without an input context, the input method doesn't see the keys.
      ::ImmAssociateContextEx(_handle, NULL, 0);
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

    bool Win32Window::HandlePointerMessage(HWND window, UINT message, WPARAM wParam) const noexcept
    {
      if (!_callbacks.OnPointer)
      {
        return false;
      }

      const UINT32 pointerId = GET_POINTERID_WPARAM(wParam);

      // The pointer's information is gone once the window loses it, so it's cancelled as it was last seen.
      if (message == WM_POINTERCAPTURECHANGED)
      {
        auto it = _activePointers.find(pointerId);
        if (it == _activePointers.end())
        {
          return false;
        }
        PointerInput input;
        input.Id = pointerId;
        input.Type = it->second;
        input.Action = PointerAction::Cancelled;
        input.Modifiers = CurrentModifiers();
        _activePointers.erase(it);
        _callbacks.OnPointer(GetWindowHandle(), input);
        return true;
      }

      POINTER_INPUT_TYPE type {};
      if (!::GetPointerType(pointerId, &type) || (type != PT_PEN && type != PT_TOUCH))
      {
        return false;
      }

      PointerInput input;
      input.Id = pointerId;
      input.Type = type == PT_PEN ? PointerType::Pen : PointerType::Touch;

      POINTER_INFO info {};
      if (type == PT_PEN)
      {
        POINTER_PEN_INFO pen {};
        if (!::GetPointerPenInfo(pointerId, &pen))
        {
          return false;
        }
        info = pen.pointerInfo;
        // Pressure from 0 to 1024, tilt in degrees from -90 to 90 and rotation in degrees from 0 to 359.
        if (pen.penMask & PEN_MASK_PRESSURE)
        {
          input.Pressure = static_cast<float>(pen.pressure) / 1024.f;
        }
        if (pen.penMask & PEN_MASK_TILT_X)
        {
          input.TiltX = static_cast<float>(pen.tiltX);
        }
        if (pen.penMask & PEN_MASK_TILT_Y)
        {
          input.TiltY = static_cast<float>(pen.tiltY);
        }
        if (pen.penMask & PEN_MASK_ROTATION)
        {
          input.Twist = static_cast<float>(pen.rotation);
        }
        input.BarrelButton = (pen.penFlags & PEN_FLAG_BARREL) != 0;
        input.Eraser = (pen.penFlags & (PEN_FLAG_ERASER | PEN_FLAG_INVERTED)) != 0;
      }
      else
      {
        POINTER_TOUCH_INFO touch {};
        if (!::GetPointerTouchInfo(pointerId, &touch))
        {
          return false;
        }
        info = touch.pointerInfo;
        if (touch.touchMask & TOUCH_MASK_CONTACTAREA)
        {
          input.Width = static_cast<float>(touch.rcContact.right - touch.rcContact.left);
          input.Height = static_cast<float>(touch.rcContact.bottom - touch.rcContact.top);
        }
        if (touch.touchMask & TOUCH_MASK_PRESSURE)
        {
          input.Pressure = static_cast<float>(touch.pressure) / 1024.f;
        }
      }

      // The position is in screen coordinates.
      POINT point = info.ptPixelLocation;
      input.ScreenX = static_cast<float>(point.x);
      input.ScreenY = static_cast<float>(point.y);
      ::ScreenToClient(window, &point);
      input.ClientX = static_cast<float>(point.x);
      input.ClientY = static_cast<float>(point.y);
      input.InContact = (info.pointerFlags & POINTER_FLAG_INCONTACT) != 0;
      input.IsPrimary = (info.pointerFlags & POINTER_FLAG_PRIMARY) != 0;
      input.Modifiers = CurrentModifiers();

      switch (message)
      {
        case WM_POINTERENTER:  input.Action = PointerAction::Entered; break;
        case WM_POINTERDOWN:   input.Action = PointerAction::Pressed; break;
        case WM_POINTERUPDATE: input.Action = PointerAction::Moved; break;
        case WM_POINTERUP:
          input.Action = (info.pointerFlags & POINTER_FLAG_CANCELED) != 0 ? PointerAction::Cancelled
                                                                          : PointerAction::Released;
          break;
        default: input.Action = PointerAction::Left; break;
      }

      // A touch is only reported while it's down: it enters and leaves with its press and release.
      if (input.Type == PointerType::Touch
          && (input.Action == PointerAction::Entered || input.Action == PointerAction::Left
              || (input.Action == PointerAction::Moved && !input.InContact)))
      {
        return true;
      }

      const bool ends = input.Action == PointerAction::Left || input.Action == PointerAction::Cancelled
                        || (input.Type == PointerType::Touch && input.Action == PointerAction::Released);
      if (ends)
      {
        _activePointers.erase(pointerId);
      }
      else
      {
        _activePointers.insert_or_assign(pointerId, input.Type);
      }
      _callbacks.OnPointer(GetWindowHandle(), input);
      return true;
    }

    void Win32Window::ProcessMessages() noexcept
    {
      // All the thread's messages: the input method and the text services (the emoji panel among them) have
      // windows of their own on it, which their messages go to.
      MSG message {};
      while (::PeekMessageW(&message, NULL, 0, 0, PM_REMOVE) != 0)
      {
        ::TranslateMessage(&message);
        ::DispatchMessageW(&message);
      }
    }

    LRESULT Win32Window::HandleMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam) const noexcept
    {
      const auto handle = GetWindowHandle();
      UpdateMouseCapture(window, message, wParam);
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
          // The pointer lock ends with the window's focus, and the system caret goes with it.
          if (message == WM_KILLFOCUS && _pointerLock)
          {
            ReleasePointerLock();
          }
          if (message == WM_KILLFOCUS)
          {
            DestroyCaret();
          }
          else
          {
            PlaceInputMethodWindows();
          }
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
        // Pens and touches; the mouse's come as the mouse messages.
        case WM_POINTERENTER:
        case WM_POINTERDOWN:
        case WM_POINTERUPDATE:
        case WM_POINTERUP:
        case WM_POINTERLEAVE:
        case WM_POINTERCAPTURECHANGED:
        {
          if (HandlePointerMessage(window, message, wParam))
          {
            return 0;
          }
          break;
        }
        // Holding a pen or a finger still doesn't wait for the system's press and hold (a right click), and
        // flicks aren't gestures: the pointers go to the window as they are.
        case WM_TABLET_QUERYSYSTEMGESTURESTATUS:
        {
          return TabletDisablePressAndHold | TabletDisableFlicks;
        }
        // Alt combinations (and F10) come as system keys; they still go on to DefWindowProc, for Alt+F4.
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
          HandleKeyDown(message, wParam, lParam);
          break;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
          if (_callbacks.OnKey)
          {
            const auto keyCode = KeyCodeFor(wParam, lParam);
            const auto key = KeyCodeToEngineKey(keyCode, lParam);
            const auto state = KeyState::Released;
            _callbacks.OnKey(handle, key, state, CurrentModifiers(), {});
            _pressedKeys.erase(keyCode);
          }
          break;
        }
        // Characters without a key press; a key press's own go with it.
        case WM_CHAR:
        {
          HandleCharacter(static_cast<wchar_t>(wParam));
          return 0;
        }
        // The host shows the text being composed; the input method still shows its candidates.
        case WM_IME_SETCONTEXT:
        {
          if (_callbacks.OnComposition)
          {
            lParam &= ~static_cast<LPARAM>(ISC_SHOWUICOMPOSITIONWINDOW);
          }
          break;
        }
        case WM_IME_STARTCOMPOSITION:
        {
          if (_callbacks.OnComposition)
          {
            _composing = true;
            _compositionEmpty = true;
            PlaceInputMethodWindows();
            ReportComposition({.Action = CompositionAction::Started});
            return 0;
          }
          break;
        }
        // Not going on to DefWindowProc, which would send the result again as characters.
        case WM_IME_COMPOSITION:
        {
          if (_callbacks.OnComposition)
          {
            HandleComposition(lParam);
            return 0;
          }
          break;
        }
        case WM_IME_ENDCOMPOSITION:
        {
          if (_callbacks.OnComposition)
          {
            if (_composing)
            {
              // The input method may end a composition it cancelled without emptying it first.
              if (!_compositionEmpty)
              {
                ReportComposition({.Action = CompositionAction::Updated});
              }
              _composing = false;
              ReportComposition({.Action = CompositionAction::Ended});
            }
            return 0;
          }
          break;
        }
        case WM_IME_NOTIFY:
        {
          if (wParam == IMN_OPENCANDIDATE)
          {
            PlaceInputMethodWindows();
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
        case WM_MOUSEHWHEEL:
        {
          if (_callbacks.OnMouseScroll)
          {
            // In notches, which smooth wheels and touchpads report fractions of. Windows is forward and right
            // positive, as the callback.
            const auto delta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
            const bool horizontal = message == WM_MOUSEHWHEEL;

            // The position is in screen coordinates.
            POINT point {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            ::ScreenToClient(window, &point);

            _callbacks.OnMouseScroll(handle, horizontal ? delta : 0, horizontal ? 0 : delta,
                                     static_cast<float>(point.x), static_cast<float>(point.y),
                                     CurrentModifiers());
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

          if (raw.header.dwType == RIM_TYPEMOUSE && _pointerLock)
          {
            // The device's movement is the raw input's; the cursor's (with the system's acceleration) is how
            // far it went from the centre, where it's put back.
            float rawX = static_cast<float>(raw.data.mouse.lLastX);
            float rawY = static_cast<float>(raw.data.mouse.lLastY);
            float x = 0;
            float y = 0;
            POINT point {};
            if (::GetCursorPos(&point))
            {
              x = static_cast<float>(point.x - _pointerLock->Centre.x);
              y = static_cast<float>(point.y - _pointerLock->Centre.y);
              if (x != 0 || y != 0)
              {
                ::SetCursorPos(_pointerLock->Centre.x, _pointerLock->Centre.y);
              }
            }
            // Raw input reports presses and releases as moves that don't move.
            if ((x != 0 || y != 0 || rawX != 0 || rawY != 0) && _callbacks.OnLockedMouseMove)
            {
              _callbacks.OnLockedMouseMove(handle, x, y, rawX, rawY, CurrentModifiers());
            }
            break;
          }
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

            // The wheel comes as WM_MOUSEWHEEL and WM_MOUSEHWHEEL, with the cursor's position.
            _callbacks.OnMouseMove(handle, x, y, clientX, clientY, CurrentModifiers());
          }
          break;
        }
#pragma endregion
        case WM_MOVE:
        {
          if (_pointerLock)
          {
            ConfinePointer();
          }
          break;
        }
        case WM_SIZE:
        {
          if (_pointerLock)
          {
            ConfinePointer();
          }
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
      // The result is whether the window was visible before, not an error.
      (void)::ShowWindow(_handle, SW_SHOW);
    }

    void Win32Window::Hide() noexcept
    {
      // The result is whether the window was visible before, not an error.
      (void)::ShowWindow(_handle, SW_HIDE);
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

      // While the window captures the mouse (a button is held), its cursor shows wherever the mouse is, and
      // no WM_SETCURSOR comes.
      if (::GetCapture() == _handle)
      {
        ApplyCursor();
        return;
      }

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

    bool Win32Window::SetPointerLocked(bool locked) noexcept
    {
      if (!locked)
      {
        if (_pointerLock)
        {
          ReleasePointerLock();
        }
        return true;
      }
      if (_pointerLock)
      {
        return true;
      }
      if (::GetFocus() != _handle)
      {
        return false;
      }

      PointerLock pointerLock;
      (void)::GetCursorPos(&pointerLock.Restore);
      _pointerLock = pointerLock;
      ConfinePointer();
      ApplyCursor();
      return true;
    }

    bool Win32Window::IsPointerLocked() const noexcept
    {
      return _pointerLock.has_value();
    }

    void Win32Window::ConfinePointer() const noexcept
    {
      RECT client {};
      if (!::GetClientRect(_handle, &client))
      {
        return;
      }
      POINT topLeft {client.left, client.top};
      POINT bottomRight {client.right, client.bottom};
      (void)::ClientToScreen(_handle, &topLeft);
      (void)::ClientToScreen(_handle, &bottomRight);
      RECT clip {topLeft.x, topLeft.y, bottomRight.x, bottomRight.y};
      (void)::ClipCursor(&clip);

      _pointerLock->Centre = {(topLeft.x + bottomRight.x) / 2, (topLeft.y + bottomRight.y) / 2};
      (void)::SetCursorPos(_pointerLock->Centre.x, _pointerLock->Centre.y);
    }

    void Win32Window::ReleasePointerLock() const noexcept
    {
      (void)::ClipCursor(nullptr);
      (void)::SetCursorPos(_pointerLock->Restore.x, _pointerLock->Restore.y);
      _pointerLock = null;
      ApplyCursor();
    }

    void Win32Window::HandleKeyDown(const UINT message, WPARAM keyCode, const LPARAM lParam) const noexcept
    {
      // Without a key handler, the characters come as text.
      if (!_callbacks.OnKey)
      {
        return;
      }

      KeyText text;
      if (keyCode == VK_PROCESSKEY)
      {
        text.HandledByInputMethod = true;
        keyCode = KeyCodeFor(keyCode, lParam);
      }

      // The characters the system translated the press to are queued after it, and go with it; a dead key
      // makes a dead character, and its accent comes with the next key's. A system key's characters stay,
      // for menu mnemonics.
      if (message == WM_KEYDOWN)
      {
        std::wstring characters;
        MSG next {};
        while (::PeekMessageW(&next, _handle, WM_CHAR, WM_DEADCHAR, PM_REMOVE) != 0)
        {
          if (next.message == WM_DEADCHAR)
          {
            text.DeadKey = true;
          }
          else if (IsTypedCharacter(static_cast<wchar_t>(next.wParam)))
          {
            characters.push_back(static_cast<wchar_t>(next.wParam));
          }
        }
        // A character outside the basic plane typed as its surrogate pair's halves, a key press for each (as
        // the emoji panel may send it), comes with the second.
        if (!characters.empty() && IS_LOW_SURROGATE(characters.front()) && _highSurrogate != 0)
        {
          characters.insert(characters.begin(), _highSurrogate);
        }
        _highSurrogate = 0;
        if (!characters.empty() && IS_HIGH_SURROGATE(characters.back()))
        {
          _highSurrogate = characters.back();
          characters.pop_back();
        }
        text.Text = ToUtf8(characters);
      }
      text.UnmodifiedText = UnmodifiedTextFor(keyCode, lParam);

      const auto key = KeyCodeToEngineKey(keyCode, lParam);
      const auto state = _pressedKeys.contains(keyCode) ? KeyState::Held : KeyState::Pressed;
      _callbacks.OnKey(GetWindowHandle(), key, state, CurrentModifiers(), text);
      _pressedKeys.emplace(keyCode);
    }

    void Win32Window::HandleCharacter(const wchar_t character) const noexcept
    {
      // A character outside the basic plane comes as its surrogate pair's halves, one at a time.
      if (IS_HIGH_SURROGATE(character))
      {
        _highSurrogate = character;
        return;
      }

      std::wstring characters;
      if (IS_LOW_SURROGATE(character))
      {
        if (_highSurrogate != 0)
        {
          characters = {_highSurrogate, character};
        }
      }
      else if (IsTypedCharacter(character))
      {
        characters.push_back(character);
      }
      _highSurrogate = 0;

      if (!characters.empty() && _callbacks.OnText)
      {
        _callbacks.OnText(GetWindowHandle(), ToUtf8(characters));
      }
    }

    void Win32Window::HandleComposition(const LPARAM changes) const noexcept
    {
      InputContext context(_handle);
      if (!context)
      {
        return;
      }

      // The result commits the composition; without one, it's text typed as it is.
      if ((changes & GCS_RESULTSTR) != 0)
      {
        auto result = ToUtf8(CompositionString(context.Get(), GCS_RESULTSTR));
        if (_composing)
        {
          _compositionEmpty = true;
          ReportComposition({.Action = CompositionAction::Committed, .Text = krys::move(result)});
        }
        else if (!result.empty() && _callbacks.OnText)
        {
          _callbacks.OnText(GetWindowHandle(), result);
        }
      }

      if (!_composing || (changes & GCS_COMPSTR) == 0)
      {
        return;
      }

      const auto text = CompositionString(context.Get(), GCS_COMPSTR);
      const auto offsets = Utf8Offsets(text);
      const auto length = static_cast<DWORD>(text.size());

      // One attribute per UTF-16 code unit, and the clauses' boundaries, from 0 to the length.
      List<BYTE> attributes;
      if ((changes & GCS_COMPATTR) != 0)
      {
        attributes = CompositionData<BYTE>(context.Get(), GCS_COMPATTR);
      }
      List<DWORD> boundaries;
      if ((changes & GCS_COMPCLAUSE) != 0)
      {
        boundaries = CompositionData<DWORD>(context.Get(), GCS_COMPCLAUSE);
      }
      if (boundaries.size() < 2)
      {
        boundaries = {0, length};
      }

      CompositionInput input {.Action = CompositionAction::Updated, .Text = ToUtf8(text)};
      for (size_t i = 0; i + 1 < boundaries.size(); ++i)
      {
        const DWORD start = (std::min)(boundaries[i], length);
        const DWORD end = (std::min)(boundaries[i + 1], length);
        if (start >= end)
        {
          continue;
        }
        const bool target =
          start < attributes.size()
          && (attributes[start] == ATTR_TARGET_CONVERTED || attributes[start] == ATTR_TARGET_NOTCONVERTED);
        input.Clauses.push_back({.Start = offsets[start], .End = offsets[end], .Target = target});
      }

      DWORD caret = length;
      if ((changes & GCS_CURSORPOS) != 0)
      {
        const LONG position = ::ImmGetCompositionStringW(context.Get(), GCS_CURSORPOS, nullptr, 0);
        caret = position < 0 ? length : (std::min)(static_cast<DWORD>(position), length);
      }
      input.Caret = offsets[caret];

      _compositionEmpty = input.Text.empty();
      ReportComposition(input);
    }

    void Win32Window::ReportComposition(const CompositionInput &input) const noexcept
    {
      if (_callbacks.OnComposition)
      {
        _callbacks.OnComposition(GetWindowHandle(), input);
      }
    }

    void Win32Window::SetTextInputEnabled(const bool enabled) noexcept
    {
      if (enabled == _textInputEnabled)
      {
        return;
      }

      if (!enabled)
      {
        CancelComposition();
        DestroyCaret();
      }
      _textInputEnabled = enabled;
      ::ImmAssociateContextEx(_handle, NULL, enabled ? IACE_DEFAULT : 0);
      if (enabled)
      {
        PlaceInputMethodWindows();
      }
    }

    bool Win32Window::IsTextInputEnabled() const noexcept
    {
      return _textInputEnabled;
    }

    void Win32Window::SetTextInputRect(const TextInputRect &rect) noexcept
    {
      _textInputRect = rect;
      PlaceInputMethodWindows();
    }

    void Win32Window::CompleteComposition() noexcept
    {
      if (!_composing)
      {
        return;
      }
      InputContext context(_handle);
      if (context)
      {
        ::ImmNotifyIME(context.Get(), NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
      }
    }

    void Win32Window::CancelComposition() noexcept
    {
      if (!_composing)
      {
        return;
      }
      // What the input method reports for it now, or later, isn't the host's.
      _composing = false;
      _compositionEmpty = true;
      InputContext context(_handle);
      if (context)
      {
        ::ImmNotifyIME(context.Get(), NI_COMPOSITIONSTR, CPS_CANCEL, 0);
      }
    }

    void Win32Window::PlaceInputMethodWindows() const noexcept
    {
      if (!_textInputEnabled || !_textInputRect)
      {
        return;
      }

      const auto &rect = *_textInputRect;
      InputContext context(_handle);
      if (context)
      {
        // The candidates go below the text, kept clear of it; some input methods place them by the
        // composition window's position instead.
        CANDIDATEFORM position {
          .dwIndex = 0,
          .dwStyle = CFS_CANDIDATEPOS,
          .ptCurrentPos = {rect.X, rect.Y + rect.Height},
        };
        ::ImmSetCandidateWindow(context.Get(), &position);
        CANDIDATEFORM exclude {
          .dwIndex = 0,
          .dwStyle = CFS_EXCLUDE,
          .ptCurrentPos = {rect.X, rect.Y},
          .rcArea = {rect.X, rect.Y, rect.X + rect.Width, rect.Y + rect.Height},
        };
        ::ImmSetCandidateWindow(context.Get(), &exclude);
        COMPOSITIONFORM composition {.dwStyle = CFS_POINT, .ptCurrentPos = {rect.X, rect.Y}};
        ::ImmSetCompositionWindow(context.Get(), &composition);
      }

      // The caret is never shown.
      if (::GetFocus() == _handle && ::CreateCaret(_handle, NULL, 1, (std::max)(rect.Height, 1)))
      {
        _hasCaret = true;
        ::SetCaretPos(rect.X, rect.Y);
      }
    }

    void Win32Window::DestroyCaret() const noexcept
    {
      if (_hasCaret)
      {
        ::DestroyCaret();
        _hasCaret = false;
      }
    }

    void Win32Window::ApplyCursor() const noexcept
    {
      if (_cursor == CursorShape::Hidden || _pointerLock)
      {
        ::SetCursor(NULL);
        return;
      }
      if (auto dragCursor = DragCursor(_cursor))
      {
        ::SetCursor(dragCursor);
        return;
      }
      ::SetCursor(::LoadCursorW(NULL, CursorShapeToResource(_cursor)));
    }
  }
}