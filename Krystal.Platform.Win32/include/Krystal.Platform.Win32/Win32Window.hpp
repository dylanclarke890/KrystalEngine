#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Platform/IWindow.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace Krys::Platform::Win32
{
  class Win32Window final : public IWindow
  {
    WindowSettings _settings;
    WindowCallbacks _callbacks;
    WNDCLASSW _class;
    HWND _handle;

  public:
    NO_COPY_MOVE(Win32Window)

    Win32Window(const WindowSettings &settings);

    ~Win32Window() noexcept override;

    void ProcessMessages() noexcept override;

    void SetTitle(const string &title) noexcept override;

    KRYS_NODISCARD const string &GetTitle() const noexcept override;

    KRYS_NODISCARD virtual WindowSize GetSize() const noexcept override;

    void Show() noexcept override;

    void Hide() noexcept override;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept override;

    void SetCallbacks(WindowCallbacks &&callbacks) noexcept override;

    LRESULT HandleMessage(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) const noexcept;

  private:
    void CreateWindowClass(HMODULE instance);
    void CreateWindowHandle(HMODULE instance);
    void RegisterRawInput() const;
  };
}