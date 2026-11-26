#pragma once

#include "Krystal.Booey/Overlay.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Web/Platform/Choosers/PopupMenu.hpp"

namespace krys::web
{
  class Element;
  class PopupMenuClient;
}

namespace krys::boo
{
  /// @brief A menu list's popup, shown by the view of its page as an overlay: an internal view whose page
  /// lists the menu's items, below the control (or above it, if there's more room there) within the viewport.
  /// Choosing an item, Escape and pressing outside the popup hide it, and go back through its client.
  class PopupMenuOverlay final : public web::PopupMenu, public Overlay
  {
  private:
    View *_owner {nullptr};
    web::PopupMenuClient *_client {nullptr};
    UniquePtr<View> _view;
    /// @brief The control the popup is for, in the owner's viewport.
    RectI _controlRect;
    /// @brief Where the popup is, in the owner's viewport.
    RectI _rect;
    /// @brief The element of each item, by its index in the list; null for items that aren't shown.
    List<RefPtr<web::Element>> _itemElements;
    int32 _highlightedIndex {-1};
    bool _visible {false};
    /// @brief Whether the control lays out right to left, so the overlay is mirrored.
    bool _rightToLeft {false};

  public:
    PopupMenuOverlay(View &owner, web::PopupMenuClient &client) noexcept;

    ~PopupMenuOverlay() noexcept override;

#pragma region PopupMenu

    void Show(const RectI &rect, web::FrameView &view, int32 selectedIndex) noexcept override;

    void Hide() noexcept final;

    void UpdateFromElement() noexcept override;

    void DisconnectClient() noexcept override;

#pragma endregion

#pragma region Overlay

    void AddRef() const noexcept final
    {
      PopupMenu::AddRef();
    }

    void SubRef() const noexcept final
    {
      PopupMenu::SubRef();
    }

    KRYS_NODISCARD uint32 GetRefCount() const noexcept final
    {
      return PopupMenu::GetRefCount();
    }

    /// @brief The view showing the popup's items.
    KRYS_NODISCARD View &ContentView() const noexcept final
    {
      return *_view;
    }

    /// @brief Where the popup is, in its owner's viewport.
    KRYS_NODISCARD const RectI &Rect() const noexcept final
    {
      return _rect;
    }

    /// @brief Highlights the item under @p point (in the popup's coordinates).
    void MouseMoved(const PointI &point) noexcept final;

    /// @brief Chooses the item under @p point (in the popup's coordinates).
    void MouseReleased(const PointI &point) noexcept final;

    /// @brief Scrolls the popup's items.
    bool MouseWheel(const PointI &point, float notchesX, float notchesY) noexcept final;

    /// @brief The arrow keys, Home and End move the highlight; Enter and Space choose the highlighted item;
    /// Escape and Tab hide the popup.
    bool KeyPressed(pal::Key key, pal::KeyModifiers modifiers) noexcept final;

#pragma endregion

    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return _visible;
    }

    /// @brief The item the keys and the mouse have chosen, by its index in the list, or -1.
    KRYS_NODISCARD int32 HighlightedIndex() const noexcept
    {
      return _highlightedIndex;
    }

    /// @brief The item at @p point (in the popup's coordinates), or -1.
    KRYS_NODISCARD int32 ItemAt(const PointI &point) const noexcept;

  private:
    /// @brief Fills the popup's page with the client's items, and places it.
    void Build() noexcept;

    void Highlight(int32 index) noexcept;

    void Choose(int32 index) noexcept;

    KRYS_NODISCARD bool IsSelectable(int32 index) const noexcept;

    /// @brief The next selectable item from @p index in @p direction, or @p index if there's none.
    KRYS_NODISCARD int32 NextSelectableIndex(int32 index, int32 direction) const noexcept;
  };
}
