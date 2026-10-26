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
#include "Krystal.PAL/Pointer.hpp"
#include "Krystal.PAL/TextInput.hpp"

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
    /// @brief The mouse's movement while the pointer is locked: the cursor's (with the system's
    /// acceleration), then the device's own (without it), then the modifiers held.
    using LockedMouseMoveHandler = Func<void(WindowHandle, float, float, float, float, KeyModifiers)>;
    using MouseButtonHandler = Func<void(WindowHandle, MouseButton, MouseButtonState, KeyModifiers)>;
    /// @brief The wheel's turn in notches (fractions of one from touchpads and smooth wheels): positive is
    /// away from the user, or to the right; then the cursor's position in client space and the modifiers
    /// held.
    using MouseScrollHandler = Func<void(WindowHandle, float, float, float, float, KeyModifiers)>;
    /// @brief A pen's or a touch's input; the mouse comes as the mouse handlers.
    using PointerHandler = Func<void(WindowHandle, const PointerInput &)>;
    using KeyHandler = Func<void(WindowHandle, Key, KeyState, KeyModifiers, const KeyText &)>;
    /// @brief Characters typed without a key press (the emoji panel, Alt and a keypad code, an input
    /// method's result outside a composition), without control characters.
    using TextHandler = Func<void(WindowHandle, const utf8_string &)>;
    /// @brief A change to the text an input method composes.
    using CompositionHandler = Func<void(WindowHandle, const CompositionInput &)>;
    using CloseHandler = Func<void(WindowHandle)>;
    using ResizeHandler = Func<void(WindowHandle, int, int)>;
    using MinimiseHandler = Func<void(WindowHandle)>;
    using RestoreHandler = Func<void(WindowHandle)>;
    using DPIChangeHandler = Func<void(WindowHandle, int)>;
    using FocusChangeHandler = Func<void(WindowHandle, bool)>;

    MouseMoveHandler OnMouseMove = nullptr;
    /// @brief Called for the mouse's moves instead of `OnMouseMove` while the pointer is locked.
    LockedMouseMoveHandler OnLockedMouseMove = nullptr;
    MouseButtonHandler OnMouseButton = nullptr;
    MouseScrollHandler OnMouseScroll = nullptr;
    /// @brief Called for pens and touches; without it, the system turns them into mouse input.
    PointerHandler OnPointer = nullptr;
    KeyHandler OnKey = nullptr;
    TextHandler OnText = nullptr;
    /// @brief Called as the input method composes text, which the host shows; without it, the input method
    /// shows its own composition window, and its result comes as `OnText`.
    CompositionHandler OnComposition = nullptr;
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

    /// @brief Locks the pointer (@p locked) or unlocks it: while it's locked, the cursor is hidden and kept
    /// in the window, and the mouse's moves come as `OnLockedMouseMove`; unlocking puts the cursor back where
    /// it was. The window unlocks it when it loses focus.
    /// @return Whether the pointer is now as asked: a window without focus can't lock it.
    virtual bool SetPointerLocked(bool locked) noexcept = 0;

    KRYS_NODISCARD virtual bool IsPointerLocked() const noexcept = 0;

    /// @brief Turns the input method on (@p enabled), for when what has the focus takes text, or off, which
    /// cancels its composition. It starts off, so key presses go to the window as they are.
    virtual void SetTextInputEnabled(bool enabled) noexcept = 0;

    KRYS_NODISCARD virtual bool IsTextInputEnabled() const noexcept = 0;

    /// @brief Sets where the text being composed is (or the caret, without a composition), so the input
    /// method's windows are placed by it and keep clear of it.
    virtual void SetTextInputRect(const TextInputRect &rect) noexcept = 0;

    /// @brief Asks the input method to commit its composition, as the host's focus moved: the result comes
    /// as `OnComposition`, usually before this returns.
    virtual void CompleteComposition() noexcept = 0;

    /// @brief Asks the input method to drop its composition, as the host dropped its own: nothing more is
    /// reported for it.
    virtual void CancelComposition() noexcept = 0;
  };
}