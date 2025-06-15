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
    HWND _windowHandle;
    Ptr<IInput> _input;
    Ptr<EventManager> _events;

  public:
    NO_COPY_MOVE(Win32Window)

    Win32Window(const WindowSettings &settings, Ptr<IInput> input, Ptr<EventManager> events) noexcept;

    ~Win32Window() noexcept override;

    virtual void ProcessMessages() noexcept override;

    virtual void SetTitle(const string &title) noexcept override;

    virtual void Show() noexcept override;

    virtual void Hide() noexcept override;

    NO_DISCARD NativeHandle GetNativeHandle() const noexcept override;

    LRESULT HandleMessage(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) noexcept;

  private:
    void Create() noexcept;

    void RegisterRawInput() const noexcept;
  };
}