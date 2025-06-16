#include "Krystal.Platform.Win32/Window.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.Platform.Win32/Input.hpp"
#include "Krystal.Platform.Win32/Utils.hpp"
#include "Krystal.Platform/Events.hpp"
#include "Krystal.Platform/IInput.hpp"
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
    using namespace Krys::Platform;

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
}

namespace Krys::Platform
{
  Expected<Unique<IWindow>> CreateWindow(const WindowSettings &settings, Ptr<IInput> input,
                                         Ptr<EventManager> events) noexcept
  {
    try
    {
      return Expected<Unique<IWindow>>(CreateUnique<Win32Window>(settings, input, events));
    }
    catch (const std::exception &e)
    {
      return Unexpected(e.what());
    }
  }

  Win32Window::Win32Window(const WindowSettings &settings, Ptr<IInput> input, Ptr<EventManager> events)
      : _handle(nullptr), _input(input), _events(events), _settings(settings)
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

  NativeHandle Win32Window::GetNativeHandle() const noexcept
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
    RECT dimensions = {0, 0, (LONG)_settings.Width, (LONG)_settings.Height};
    int styles = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
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

  LRESULT Win32Window::HandleMessage(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) noexcept
  {
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
        _events->Enqueue(CreateUnique<QuitEvent>(NativeHandle {handle}));
        return 0;
      }
      default:
      {
        auto handled =
          static_cast<Win32Input *>(_input)->HandleWindowMessage(message, wParam, lParam, handle);
        if (handled)
          return 0;
        break;
      }
    }

    return ::DefWindowProc(handle, message, wParam, lParam);
  }

  void Win32Window::SetTitle(const string &title) noexcept
  {
    auto result = ::SetWindowText(_handle, ToWideString(title).c_str());
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
}