#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.PAL/IWindow.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
#endif

namespace krys::pal::win32
{
  class DirectManipulationPan;
  class DropTarget;
  class UIAutomation;

  class Win32Window final : public IWindow
  {
  private:
    WindowSettings _settings;
    WindowCallbacks _callbacks;
    WNDCLASSW _class;
    HWND _handle;
    CursorShape _cursor {CursorShape::Arrow};
    /// @brief The image shown as the cursor in place of `_cursor`, and the system cursor made from it.
    RefPtr<CursorImage> _cursorImage;
    HCURSOR _imageCursor {nullptr};
    /// @brief The pens and touches in range, by id, so a capture change can cancel them.
    mutable HashMap<uint32, PointerType> _activePointers;

    /// @brief The pointer lock: where the cursor is kept (the client area's centre) and where it goes back
    /// to, in screen coordinates.
    struct PointerLock
    {
      POINT Centre {};
      POINT Restore {};
    };

    mutable Maybe<PointerLock> _pointerLock;
    /// @brief The keys down, by virtual key code, to tell presses from repeats.
    mutable HashSet<WPARAM> _pressedKeys;

    bool _textInputEnabled {false};
    Maybe<TextInputRect> _textInputRect;
    /// @brief Whether the input method is composing, as last reported; and whether the text it composes, as
    /// last reported, is empty (so an ended composition can be reported as emptied first).
    mutable bool _composing {false};
    mutable bool _compositionEmpty {true};
    /// @brief The first half of a surrogate pair that came as a character without a key press, until its
    /// second half comes.
    mutable wchar_t _highSurrogate {0};
    /// @brief Whether the window has made the system caret, for the input method's windows.
    mutable bool _hasCaret {false};
    /// @brief The window's drop target, while the host takes drops.
    DropTarget *_dropTarget {nullptr};
    /// @brief The window's DirectManipulation viewport, while the host takes touchpad pans.
    DirectManipulationPan *_touchpadPan {nullptr};
    /// @brief The display the refresh rate was last read for, and its rate; a display change clears them.
    mutable HMONITOR _refreshRateMonitor {nullptr};
    mutable Maybe<uint32> _refreshRate;
    /// @brief The display the colours were last read for, and what it shows; a display change clears them.
    mutable HMONITOR _colourMonitor {nullptr};
    mutable DisplayColour _colour;
    /// @brief The display the window was on when it last moved, to tell when it moves to another.
    mutable HMONITOR _monitor {nullptr};
    /// @brief Exposes the host's accessibility tree to UI Automation, while the host gives one.
    UniquePtr<UIAutomation> _uiAutomation;

  public:
    Win32Window(const WindowSettings &settings);

    ~Win32Window() noexcept override;

    void ProcessMessages() noexcept override;

    void SetTitle(const string &title) noexcept override;

    KRYS_NODISCARD const string &GetTitle() const noexcept override;

    KRYS_NODISCARD virtual WindowSize GetSize() const noexcept override;

    KRYS_NODISCARD WindowPosition ClientOriginOnScreen() const noexcept override;

    KRYS_NODISCARD Maybe<uint32> DisplayRefreshRate() const noexcept override;

    KRYS_NODISCARD DisplayColour GetDisplayColour() const noexcept override;

    void Show() noexcept override;

    void Hide() noexcept override;

    KRYS_NODISCARD WindowHandle GetWindowHandle() const noexcept override;

    void SetCallbacks(WindowCallbacks &&callbacks) noexcept override;

    void SetCursor(CursorShape cursor) noexcept override;
    void SetCursorImage(Ref<CursorImage> image) noexcept override;

    KRYS_NODISCARD CursorShape GetCursor() const noexcept override;
    KRYS_NODISCARD CursorImage *GetCursorImage() const noexcept override;

    bool SetPointerLocked(bool locked) noexcept override;

    KRYS_NODISCARD bool IsPointerLocked() const noexcept override;

    void SetTextInputEnabled(bool enabled) noexcept override;

    KRYS_NODISCARD bool IsTextInputEnabled() const noexcept override;

    void SetTextInputRect(const TextInputRect &rect) noexcept override;

    void CompleteComposition() noexcept override;

    void CancelComposition() noexcept override;

    void SetDropOperation(Maybe<DropOperation> operation) noexcept override;

    void SetAccessibilityTree(IAccessibilityTree *tree) noexcept override;

    KRYS_NODISCARD bool IsAccessibilityActive() const noexcept override;

    void RaiseAccessibilityEvent(AccessibilityNodeID node, AccessibilityEvent event) noexcept override;

    void RaiseAccessibilityTextChange(AccessibilityNodeID node, AccessibilityTextChange change, uint64 offset,
                                      utf8_stringview text) noexcept override;

    void AccessibilityNodeRemoved(AccessibilityNodeID node) noexcept override;

    LRESULT HandleMessage(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) const noexcept;

  private:
    void CreateWindowClass(HMODULE instance);
    void CreateWindowHandle(HMODULE instance);
    void RegisterRawInput() const;
    /// @brief Reports a pen's or a touch's pointer message.
    /// @return Whether it was a pen's or a touch's, which the system then doesn't turn into mouse input.
    bool HandlePointerMessage(HWND window, UINT message, WPARAM wParam) const noexcept;
    /// @brief Shows the window's cursor, as the mouse is over the client area (none while the pointer is
    /// locked).
    void ApplyCursor() const noexcept;
    /// @brief Applies the cursor now if it's showing: while the window captures the mouse, or under it.
    void CursorChanged() const noexcept;
    /// @brief Destroys the system cursor made from the cursor image, and forgets the image.
    void ClearCursorImage() noexcept;
    /// @brief Keeps the cursor in the client area, at its centre, while the pointer is locked.
    void ConfinePointer() const noexcept;
    /// @brief Unlocks the pointer, putting the cursor back where it was.
    void ReleasePointerLock() const noexcept;
    /// @brief Reports a key press, with the characters the system translated it to.
    void HandleKeyDown(UINT message, WPARAM keyCode, LPARAM lParam) const noexcept;
    /// @brief Reports a character that came without a key press.
    void HandleCharacter(wchar_t character) const noexcept;
    /// @brief Reports the input method's composition changing, as @p changes (`GCS_*` flags) say.
    void HandleComposition(LPARAM changes) const noexcept;
    /// @brief Reports a change to the composition, if the host takes them.
    void ReportComposition(const CompositionInput &input) const noexcept;
    /// @brief Places the input method's windows by the text input rect, and the system caret (which some
    /// input methods and accessibility tools follow) at it.
    void PlaceInputMethodWindows() const noexcept;
    /// @brief Destroys the system caret, if the window made it.
    void DestroyCaret() const noexcept;
    /// @brief Registers the window's drop target if the host takes drops, or revokes it if not.
    void UpdateDropTarget() noexcept;
    /// @brief Revokes the window's drop target, if it has one.
    void RevokeDropTarget() noexcept;
    /// @brief Makes the window's DirectManipulation viewport if the host takes touchpad pans, or destroys it
    /// if not.
    void UpdateTouchpadPan() noexcept;
    /// @brief Destroys the window's DirectManipulation viewport, if it has one.
    void DestroyTouchpadPan() noexcept;
    /// @brief Reports a step of a touchpad's pan, where the cursor is.
    void ReportTouchpadPan(TouchpadPanPhase phase, float deltaX, float deltaY) const noexcept;
  };
}