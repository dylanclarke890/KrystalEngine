#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Platform/IInput.hpp"
#include "Krystal.Platform/IWindow.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace Krys::Platform
{
  class Win32Window final : public IWindow
  {
    WNDCLASS _class;
    HWND _handle;
    Ptr<IInput> _input;
    Ptr<EventManager> _events;
    WindowSettings _settings;

  public:
    NO_COPY_MOVE(Win32Window)

    Win32Window(const WindowSettings &settings, Ptr<IInput> input, Ptr<EventManager> events);

    ~Win32Window() noexcept override;

    void ProcessMessages() noexcept override;

    void SetTitle(const string &title) noexcept override;

    NO_DISCARD const string &GetTitle() const noexcept override;

    void Show() noexcept override;

    void Hide() noexcept override;

    NO_DISCARD NativeHandle GetNativeHandle() const noexcept override;

    LRESULT HandleMessage(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) noexcept;

  private:
    void CreateWindowClass(HMODULE instance);
    void CreateWindowHandle(HMODULE instance);
    void RegisterRawInput() const;
  };
}