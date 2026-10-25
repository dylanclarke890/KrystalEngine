#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.PAL/IWindow.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif

namespace krys::pal::win32
{
  class Win32Window final : public IWindow
  {
  private:
    WindowSettings _settings;
    WindowCallbacks _callbacks;
    WNDCLASSW _class;
    HWND _handle;
    CursorShape _cursor {CursorShape::Arrow};
    /// @brief The pens and touches in range, by id, so a capture change can cancel them.
    mutable HashMap<uint32, PointerType> _activePointers;

  public:
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

    void SetCursor(CursorShape cursor) noexcept override;

    KRYS_NODISCARD CursorShape GetCursor() const noexcept override;

    LRESULT HandleMessage(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) const noexcept;

  private:
    void CreateWindowClass(HMODULE instance);
    void CreateWindowHandle(HMODULE instance);
    void RegisterRawInput() const;
    /// @brief Reports a pen's or a touch's pointer message.
    /// @return Whether it was a pen's or a touch's, which the system then doesn't turn into mouse input.
    bool HandlePointerMessage(HWND window, UINT message, WPARAM wParam) const noexcept;
    /// @brief Shows the window's cursor, as the mouse is over the client area.
    void ApplyCursor() const noexcept;
  };
}