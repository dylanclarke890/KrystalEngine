#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Utils/NativeHandle.hpp"
#include "Krystal.PAL/Accessibility.hpp"
#include "Krystal.PAL/CursorShapes.hpp"
#include "Krystal.PAL/Display.hpp"
#include "Krystal.PAL/DragAndDrop.hpp"
#include "Krystal.PAL/KeyModifiers.hpp"
#include "Krystal.PAL/Keys.hpp"
#include "Krystal.PAL/MouseButtons.hpp"
#include "Krystal.PAL/Pointer.hpp"
#include "Krystal.PAL/TextInput.hpp"
#include "Krystal.PAL/TouchpadPan.hpp"

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

  /// @brief A point on the screen, in its pixels.
  struct WindowPosition
  {
    int32 X {0}, Y {0};
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
    /// @brief A step of a precision touchpad's two-finger pan.
    using TouchpadPanHandler = Func<void(WindowHandle, const TouchpadPanInput &)>;
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
    using DisplayChangeHandler = Func<void(WindowHandle)>;
    /// @brief A drag from another application over the window.
    using DragHandler = Func<void(WindowHandle, const DragInput &)>;

    MouseMoveHandler OnMouseMove = nullptr;
    /// @brief Called for the mouse's moves instead of `OnMouseMove` while the pointer is locked.
    LockedMouseMoveHandler OnLockedMouseMove = nullptr;
    MouseButtonHandler OnMouseButton = nullptr;
    MouseScrollHandler OnMouseScroll = nullptr;
    /// @brief Called for a precision touchpad's pans, with their phases and momentum; without it (or where
    /// the system can't report them), they come as `OnMouseScroll`.
    TouchpadPanHandler OnTouchpadPan = nullptr;
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
    /// @brief Called when the window moves to another display, or its display's settings change: what
    /// `IWindow::DisplayRefreshRate()` and `IWindow::GetDisplayColour()` say may have changed.
    DisplayChangeHandler OnDisplayChange = nullptr;
    /// @brief Called as drags from other applications come over the window, move, leave and drop; the host
    /// answers with `IWindow::SetDropOperation()`. Without it, the window takes no drops.
    DragHandler OnDrag = nullptr;
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

    /// @brief Where the client area's top-left corner is on the screen.
    KRYS_NODISCARD virtual WindowPosition ClientOriginOnScreen() const noexcept = 0;

    /// @brief The refresh rate of the display the window is on, in hertz, or null if the system doesn't say.
    KRYS_NODISCARD virtual Maybe<uint32> DisplayRefreshRate() const noexcept = 0;

    /// @brief What colours the display the window is on shows: its gamut and whether it's showing high
    /// dynamic range.
    KRYS_NODISCARD virtual DisplayColour GetDisplayColour() const noexcept = 0;

    virtual void Show() noexcept = 0;

    virtual void Hide() noexcept = 0;

    virtual void SetCallbacks(WindowCallbacks &&callbacks) noexcept = 0;

    /// @brief Sets the cursor shown while the mouse is over the window's client area; it changes at once if
    /// the mouse is there now.
    virtual void SetCursor(CursorShape cursor) noexcept = 0;

    /// @brief Shows @p image as the cursor, as `SetCursor` does a system one, until a cursor is set again.
    virtual void SetCursorImage(Ref<CursorImage> image) noexcept = 0;

    /// @brief The system cursor last set; an image set since shows in its place.
    KRYS_NODISCARD virtual CursorShape GetCursor() const noexcept = 0;

    /// @brief The image shown as the cursor, or null for a system cursor.
    KRYS_NODISCARD virtual CursorImage *GetCursorImage() const noexcept = 0;

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

    /// @brief Sets what a drop of the drag over the window would do where the cursor is, as the host answers
    /// the drag's input (none until it does, and after the drag leaves or drops): the drag's source hears it
    /// at the drag's next move, and at the drop. A drop is reported only if the last answer was an operation.
    virtual void SetDropOperation(Maybe<DropOperation> operation) noexcept = 0;

    /// @brief Sets the tree the window exposes to assistive technology (null for none, the default), which
    /// the window asks about only when assistive technology does. The host keeps it alive while it's set.
    virtual void SetAccessibilityTree(IAccessibilityTree *tree) noexcept = 0;

    /// @brief Whether assistive technology has asked for the window's accessibility tree, so the host's
    /// events reach it.
    KRYS_NODISCARD virtual bool IsAccessibilityActive() const noexcept = 0;

    /// @brief Tells assistive technology @p event happened to @p node.
    virtual void RaiseAccessibilityEvent(AccessibilityNodeID node, AccessibilityEvent event) noexcept = 0;

    /// @brief Tells assistive technology @p text was inserted into or deleted from @p node's text at
    /// @p offset (in bytes of its UTF-8 text).
    virtual void RaiseAccessibilityTextChange(AccessibilityNodeID node, AccessibilityTextChange change,
                                              uint64 offset, utf8_stringview text) noexcept = 0;

    /// @brief Tells the window @p node left the tree, so it lets go of what it made for it.
    virtual void AccessibilityNodeRemoved(AccessibilityNodeID node) noexcept = 0;
  };
}