#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Utils/NativeHandle.hpp"
#include "Krystal.PAL/CursorShapes.hpp"
#include "Krystal.PAL/KeyModifiers.hpp"
#include "Krystal.PAL/Keys.hpp"
#include "Krystal.PAL/MouseButtons.hpp"

namespace krys::pal
{
  struct WindowSettings;
  struct WindowCallbacks;
  class IWindow;

  Expected<UniquePtr<IWindow>> CreateWindow(const WindowSettings &settings) noexcept;

  struct WindowSize
  {
    uint32 Width {800}, Height {600};
  };

  struct WindowSizeBounds
  {
    /// @brief Whether the window can be resized. If false, Min and Max are ignored.
    bool Resizable {true};

    /// @brief The minimum size of the window.
    WindowSize Min {100, 100};

    /// @brief The maximum size of the window.
    WindowSize Max {1'920, 1'080};
  };

  struct WindowSettings
  {
    /// @brief The title of the window.
    string Title {};

    /// @brief The size of the window.
    WindowSize Size {800, 600};

    /// @brief The bounds for the window size.
    WindowSizeBounds SizeBounds {};

    bool Visible {true};

    /// @brief The initial vsync state.
    bool VSync {true};
  };

  using WindowHandle = NativeHandle;

  struct WindowCallbacks
  {
    using MouseMoveHandler = Func<void(WindowHandle, float, float, float, float, KeyModifiers)>;
    using MouseButtonHandler = Func<void(WindowHandle, MouseButton, MouseButtonState, KeyModifiers)>;
    /// @brief The wheel's turn in notches (fractions of one from touchpads and smooth wheels): positive is
    /// away from the user, or to the right; then the cursor's position in client space and the modifiers
    /// held.
    using MouseScrollHandler = Func<void(WindowHandle, float, float, float, float, KeyModifiers)>;
    using KeyHandler = Func<void(WindowHandle, Key, KeyState, KeyModifiers, const KeyText &)>;
    using CloseHandler = Func<void(WindowHandle)>;
    using ResizeHandler = Func<void(WindowHandle, int, int)>;
    using MinimiseHandler = Func<void(WindowHandle)>;
    using RestoreHandler = Func<void(WindowHandle)>;
    using DPIChangeHandler = Func<void(WindowHandle, int)>;
    using FocusChangeHandler = Func<void(WindowHandle, bool)>;

    MouseMoveHandler OnMouseMove = nullptr;
    MouseButtonHandler OnMouseButton = nullptr;
    MouseScrollHandler OnMouseScroll = nullptr;
    KeyHandler OnKey = nullptr;
    CloseHandler OnClose = nullptr;
    ResizeHandler OnResize = nullptr;
    MinimiseHandler OnMinimise = nullptr;
    RestoreHandler OnRestore = nullptr;
    DPIChangeHandler OnDPIChange = nullptr;
    /// @brief Called when the window gains (true) or loses (false) keyboard focus.
    FocusChangeHandler OnFocusChange = nullptr;
  };

  class IWindow
  {
  protected:
    IWindow() noexcept = default;

  public:
    virtual ~IWindow() noexcept = default;

    KRYS_NODISCARD virtual WindowHandle GetWindowHandle() const noexcept = 0;

    virtual void ProcessMessages() noexcept = 0;

    virtual void SetTitle(const string &title) noexcept = 0;

    KRYS_NODISCARD virtual const string &GetTitle() const noexcept = 0;

    KRYS_NODISCARD virtual WindowSize GetSize() const noexcept = 0;

    virtual void Show() noexcept = 0;

    virtual void Hide() noexcept = 0;

    virtual void SetCallbacks(WindowCallbacks &&callbacks) noexcept = 0;

    /// @brief Sets the cursor shown while the mouse is over the window's client area; it changes at once if
    /// the mouse is there now.
    virtual void SetCursor(CursorShape cursor) noexcept = 0;

    KRYS_NODISCARD virtual CursorShape GetCursor() const noexcept = 0;
  };
}