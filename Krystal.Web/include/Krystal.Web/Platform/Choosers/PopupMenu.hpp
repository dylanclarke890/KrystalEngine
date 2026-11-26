#pragma once

#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"

namespace krys::web
{
  class FrameView;

  /// @brief The open list of a menu list (a drop-down `<select>`), made by the host
  /// (`WebClient::CreatePopupMenu`) and drawn however it likes. It reads the items from its
  /// PopupMenuClient and tells it what the user chose.
  class PopupMenu : public RefCounted<PopupMenu>
  {
  public:
    virtual ~PopupMenu() noexcept = default;

    /// @brief Shows the menu for a control at @p rect, in the coordinates of @p view's contents, with the
    /// item at @p selectedIndex chosen.
    virtual void Show(const RectI &rect, FrameView &view, int32 selectedIndex) noexcept = 0;

    /// @brief Closes the menu; the client is told through PopupMenuClient::PopupDidHide.
    virtual void Hide() noexcept = 0;

    /// @brief Reads the items from the client again, as they changed while the menu was open.
    virtual void UpdateFromElement() noexcept = 0;

    /// @brief Forgets the client, which is going away.
    virtual void DisconnectClient() noexcept = 0;
  };
}
