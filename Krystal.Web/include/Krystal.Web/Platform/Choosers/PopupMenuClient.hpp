#pragma once

#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Web/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Web/Platform/Choosers/PopupMenuStyle.hpp"

namespace krys::web
{
  /// @brief What a PopupMenu shows and tells about the user's choice: a menu list (`RenderMenuList`).
  /// Items are counted by list index: options, group labels (`<optgroup>`) and separators (`<hr>`) in order.
  class PopupMenuClient
  {
  public:
    virtual ~PopupMenuClient() noexcept = default;

    /// @brief Selects the option at @p listIndex as the user's choice, firing `input` and `change` if
    /// @p fireEvents.
    virtual void ValueChanged(uint32 listIndex, bool fireEvents = true) noexcept = 0;

    /// @brief The text an item shows: an option's, or a group's label.
    KRYS_NODISCARD virtual utf8_string ItemText(uint32 listIndex) const noexcept = 0;

    /// @brief An item's tooltip: its `title`.
    KRYS_NODISCARD virtual utf8_string ItemToolTip(uint32 listIndex) const noexcept = 0;

    /// @brief An item's accessible name when its text isn't it: its `aria-label`, or empty.
    KRYS_NODISCARD virtual utf8_string ItemAccessibilityText(uint32 listIndex) const noexcept = 0;

    /// @brief Whether an item can be chosen: an option that isn't disabled and isn't in a disabled group.
    KRYS_NODISCARD virtual bool ItemIsEnabled(uint32 listIndex) const noexcept = 0;

    /// @brief How to draw an item: its colours, font, visibility and direction.
    KRYS_NODISCARD virtual PopupMenuStyle ItemStyle(uint32 listIndex) const noexcept = 0;

    /// @brief How to draw the menu: the control's colours, font and direction, and the menu's size.
    KRYS_NODISCARD virtual PopupMenuStyle MenuStyle() const noexcept = 0;

    /// @brief The space to leave before and after the items' text, to line it up with the control's.
    KRYS_NODISCARD virtual LayoutUnit ClientPaddingLeft() const noexcept = 0;

    KRYS_NODISCARD virtual LayoutUnit ClientPaddingRight() const noexcept = 0;

    /// @brief The number of items.
    KRYS_NODISCARD virtual int32 ListSize() const noexcept = 0;

    /// @brief The list index of the selected option, or -1.
    KRYS_NODISCARD virtual int32 SelectedIndex() const noexcept = 0;

    /// @brief Tells the menu list its menu has closed.
    virtual void PopupDidHide() noexcept = 0;

    /// @brief Whether an item is a separator (`<hr>`).
    KRYS_NODISCARD virtual bool ItemIsSeparator(uint32 listIndex) const noexcept = 0;

    /// @brief Whether an item is a group's label (`<optgroup>`).
    KRYS_NODISCARD virtual bool ItemIsLabel(uint32 listIndex) const noexcept = 0;

    /// @brief Whether an item is the selected option.
    KRYS_NODISCARD virtual bool ItemIsSelected(uint32 listIndex) const noexcept = 0;

    /// @brief Shows an option's text in the control without selecting it, as a preview while the menu is
    /// open.
    virtual void SetTextFromItem(uint32 listIndex) noexcept = 0;
  };
}
