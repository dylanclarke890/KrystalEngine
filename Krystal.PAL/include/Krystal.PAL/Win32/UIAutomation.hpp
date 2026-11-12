#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.PAL/Accessibility.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace krys::pal::win32
{
  class UIAElement;

  /// @brief Exposes a window's accessibility tree to UI Automation (and so to Narrator and other screen
  /// readers): it answers `WM_GETOBJECT` with the window's root element, makes an element for each node
  /// assistive technology asks about, and raises the tree's events on them.
  /// @note The root element is the window's client area; the tree's root is its only child. Elements are
  /// called on the window's thread (UI Automation marshals its calls through COM).
  class UIAutomation
  {
    KRYS_NON_COPY_MOVABLE(UIAutomation);

  private:
    HWND _window {nullptr};
    IAccessibilityTree &_tree;
    /// @brief The elements made for the tree's nodes, by node, each holding a reference.
    HashMap<AccessibilityNodeID, UIAElement *> _elements;
    /// @brief The window's element, made on the first `WM_GETOBJECT`.
    UIAElement *_root {nullptr};

  public:
    UIAutomation(HWND window, IAccessibilityTree &tree) noexcept;
    /// @brief Disconnects the elements, which fail every call from then on.
    ~UIAutomation() noexcept;

    /// @brief Answers `WM_GETOBJECT`, if it asks for UI Automation's root element.
    KRYS_NODISCARD Maybe<LRESULT> HandleGetObject(WPARAM wParam, LPARAM lParam) noexcept;
    /// @brief Lets UI Automation go of the window's elements, as it's destroyed.
    void WindowDestroyed() noexcept;

    /// @brief Whether a UI Automation client is listening for events.
    KRYS_NODISCARD static bool ClientsAreListening() noexcept;
    void RaiseEvent(AccessibilityNodeID node, AccessibilityEvent event) noexcept;
    void RaiseTextChange(AccessibilityNodeID node) noexcept;
    /// @brief Disconnects @p node's element, if it has one.
    void NodeRemoved(AccessibilityNodeID node) noexcept;

    // For the elements.

    KRYS_NODISCARD HWND Window() const noexcept
    {
      return _window;
    }
    KRYS_NODISCARD IAccessibilityTree &Tree() const noexcept
    {
      return _tree;
    }
    /// @brief The window's element.
    KRYS_NODISCARD UIAElement &Root() noexcept;
    /// @brief @p node's element (made if it has none); the window's for 0.
    KRYS_NODISCARD UIAElement &Element(AccessibilityNodeID node) noexcept;

  private:
    void Disconnect(UIAElement &element) noexcept;
  };
}
