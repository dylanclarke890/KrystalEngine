#pragma once

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo
{
  class LocalFrameView;

  /// @brief A menu the host shows for a menu list: its options, below or above the control.
  class PopupMenu : public RefCounted<PopupMenu>
  {
  public:
    virtual ~PopupMenu() noexcept = default;

    /// @brief Shows the menu for a control at @p rect, in the coordinates of @p view's contents, with the
    /// item at @p selectedIndex chosen.
    virtual void Show(const IntRect &rect, LocalFrameView &view, int32 selectedIndex) noexcept = 0;

    virtual void Hide() noexcept = 0;

    /// @brief Updates the menu's items from its client.
    virtual void UpdateFromElement() noexcept = 0;

    /// @brief Forgets the client, which is going away.
    virtual void DisconnectClient() noexcept = 0;
  };
}
