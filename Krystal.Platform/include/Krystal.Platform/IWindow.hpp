#pragma once

#include "Krystal.Lib/Mixins/NonCopyMovable.hpp"
#include "Krystal.Lib/NativeHandle.hpp"
#include "Krystal.Lib/Types/Expected.hpp"
#include "Krystal.Lib/Types/Func.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/SmartPointers.hpp"
#include "Krystal.Platform/Keys.hpp"
#include "Krystal.Platform/MouseButtons.hpp"

namespace Krys::Platform
{
  struct WindowSettings;
  struct WindowCallbacks;
  class IWindow;

  Expected<Unique<IWindow>> CreateWindow(const WindowSettings &settings) noexcept;

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
    using MouseMoveHandler = Func<void(WindowHandle, float, float, float, float)>;
    using MouseButtonHandler = Func<void(WindowHandle, MouseButton, MouseButtonState)>;
    using MouseScrollHandler = Func<void(WindowHandle, float)>;
    using KeyHandler = Func<void(WindowHandle, Key, KeyState)>;
    using CloseHandler = Func<void(WindowHandle)>;
    using ResizeHandler = Func<void(WindowHandle, int, int)>;
    using MinimiseHandler = Func<void(WindowHandle)>;
    using RestoreHandler = Func<void(WindowHandle)>;
    using DPIChangeHandler = Func<void(WindowHandle, int)>;

    MouseMoveHandler OnMouseMove = nullptr;
    MouseButtonHandler OnMouseButton = nullptr;
    MouseScrollHandler OnMouseScroll = nullptr;
    KeyHandler OnKey = nullptr;
    CloseHandler OnClose = nullptr;
    ResizeHandler OnResize = nullptr;
    MinimiseHandler OnMinimise = nullptr;
    RestoreHandler OnRestore = nullptr;
    DPIChangeHandler OnDPIChange = nullptr;
  };

  class IWindow : NonCopyMovable<IWindow>
  {
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

  protected:
    IWindow() noexcept = default;
  };
}