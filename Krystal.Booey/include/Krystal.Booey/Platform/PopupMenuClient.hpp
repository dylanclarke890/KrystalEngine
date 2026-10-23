#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Platform/PopupMenuStyle.hpp"
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
