/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"
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
