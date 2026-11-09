/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2006, 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Booey/Platform/Choosers/PopupMenuStyle.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief What a popup menu shows and who it tells about the user's choice: a menu list.
  class PopupMenuClient
  {
  public:
    virtual ~PopupMenuClient() noexcept = default;

    virtual void ValueChanged(uint32 listIndex, bool fireEvents = true) noexcept = 0;
    virtual void SelectionChanged(uint32 listIndex, bool fireEvents = true) noexcept = 0;
    virtual void SelectionCleared() noexcept = 0;

    KRYS_NODISCARD virtual utf8_string ItemText(uint32 listIndex) const noexcept = 0;
    KRYS_NODISCARD virtual utf8_string ItemLabel(uint32 listIndex) const noexcept = 0;
    KRYS_NODISCARD virtual utf8_string ItemIcon(uint32 listIndex) const noexcept = 0;
    KRYS_NODISCARD virtual utf8_string ItemToolTip(uint32 listIndex) const noexcept = 0;
    KRYS_NODISCARD virtual utf8_string ItemAccessibilityText(uint32 listIndex) const noexcept = 0;
    KRYS_NODISCARD virtual bool ItemIsEnabled(uint32 listIndex) const noexcept = 0;
    KRYS_NODISCARD virtual PopupMenuStyle ItemStyle(uint32 listIndex) const noexcept = 0;
    KRYS_NODISCARD virtual PopupMenuStyle MenuStyle() const noexcept = 0;
    KRYS_NODISCARD virtual int32 ClientInsetLeft() const noexcept = 0;
    KRYS_NODISCARD virtual int32 ClientInsetRight() const noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit ClientPaddingLeft() const noexcept = 0;
    KRYS_NODISCARD virtual LayoutUnit ClientPaddingRight() const noexcept = 0;
    KRYS_NODISCARD virtual int32 ListSize() const noexcept = 0;
    KRYS_NODISCARD virtual int32 SelectedIndex() const noexcept = 0;
    virtual void PopupDidHide() noexcept = 0;
    KRYS_NODISCARD virtual bool ItemIsSeparator(uint32 listIndex) const noexcept = 0;
    KRYS_NODISCARD virtual bool ItemIsLabel(uint32 listIndex) const noexcept = 0;
    KRYS_NODISCARD virtual bool ItemIsSelected(uint32 listIndex) const noexcept = 0;
    KRYS_NODISCARD virtual bool ShouldPopOver() const noexcept = 0;
    virtual void SetTextFromItem(uint32 listIndex) noexcept = 0;

    virtual void ListBoxSelectItem(int32 listIndex, bool allowMultiplySelections, bool shift,
                                   bool fireOnChangeNow = true) noexcept
    {
      (void)listIndex;
      (void)allowMultiplySelections;
      (void)shift;
      (void)fireOnChangeNow;
      krys_debug_assert_msg(false, "Not reached");
    }

    KRYS_NODISCARD virtual bool Multiple() const noexcept
    {
      krys_debug_assert_msg(false, "Not reached");
      return false;
    }
  };
}
