#include "Krystal.Platform.Win32/Window.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.Platform.Win32/Input.hpp"
#include "Krystal.Platform/Events.hpp"
#include "Krystal.Platform/IInput.hpp"
#include "Krystal.Platform/IWindow.hpp"
#include "Krystal.Platform/Platform.hpp"

#ifndef UNICODE
  #define UNICODE
#endif

#include <cassert>
#include <hidusage.h>
#include <windows.h>

#include <libloaderapi.h>
#include <string>

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
#ifdef CreateWindow
  #pragma push_macro("CreateWindow")
  #undef CreateWindow
#endif

  Unique<IWindow> CreateWindow(const WindowSettings &settings, Ptr<IInput> input,
                               Ptr<EventManager> events) noexcept
  {
    return Unique<IWindow>(new Win32Window(settings, input, events));
  }

#pragma pop_macro("CreateWindow")

  Win32Window::Win32Window(const WindowSettings &settings, Ptr<IInput> input,
                           Ptr<EventManager> events) noexcept
      : IWindow(settings), _windowHandle(nullptr), _input(input), _events(events)
  {
    Create();
  }

  Win32Window::~Win32Window() noexcept
  {
  }

  void Win32Window::Create() noexcept
  {
    const auto instance = ::GetModuleHandle(NULL);

    WNDCLASS windowClass = {};
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.hInstance = instance;
    windowClass.lpfnWndProc = &WindowProc;
    windowClass.hbrBackground = NULL;
    windowClass.hCursor = ::LoadCursor(instance, IDC_ARROW);
    windowClass.cbWndExtra = sizeof(Win32Window *);
    windowClass.lpszClassName = L"KrystalWindowClass";

    {
      auto result = ::RegisterClass(&windowClass);
      assert(result);
    }

    RECT dimensions = {0, 0, (LONG)_settings.Width, (LONG)_settings.Height};
    int styles = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;

    {
      auto result = ::AdjustWindowRect(&dimensions, styles, 0);
      assert(result);
    }

    int width = dimensions.right - dimensions.left;
    int height = dimensions.bottom - dimensions.top;

    _windowHandle =
      ::CreateWindowEx(0, windowClass.lpszClassName, ToWideString(_settings.Title).c_str(), styles,
                       CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, instance, this);
    assert(_windowHandle != nullptr);

    {
      auto result = ::UpdateWindow(_windowHandle);
      assert(result);
    }

    RegisterRawInput();
  }

  void Win32Window::RegisterRawInput() const noexcept
  {
    // TODO: account for dpi?

    RAWINPUTDEVICE rid {};
    rid.usUsagePage = HID_USAGE_PAGE_GENERIC;
    rid.usUsage = HID_USAGE_GENERIC_MOUSE;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = _windowHandle;

    auto result = ::RegisterRawInputDevices(&rid, 1, sizeof(rid));
    assert(result);
  }

  void Win32Window::ProcessMessages() noexcept
  {
    MSG message {};
    while (::PeekMessage(&message, _windowHandle, 0, 0, PM_REMOVE) != 0)
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
        return TRUE;
      }
      default:
      {
        auto handled =
          static_cast<Win32Input *>(_input)->HandleWindowMessage(message, wParam, lParam, handle);
        if (handled)
          return TRUE;
        break;
      }
    }

    return ::DefWindowProc(handle, message, wParam, lParam);
  }

  void Win32Window::SetTitle(const string &title) noexcept
  {
    _settings.Title = title;
    auto result = ::SetWindowText(_windowHandle, ToWideString(title).c_str());
    assert(result);
  }

  void Win32Window::Show() noexcept
  {
    auto result = ::ShowWindow(_windowHandle, SW_SHOW);
    assert(result);
  }

  void Win32Window::Hide() noexcept
  {
    auto result = ::ShowWindow(_windowHandle, SW_HIDE);
    assert(result);
  }

  NativeHandle Win32Window::GetNativeHandle() const noexcept
  {
    return NativeHandle {_windowHandle};
  }
}