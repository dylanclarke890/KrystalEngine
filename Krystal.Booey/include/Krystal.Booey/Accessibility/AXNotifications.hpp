/*
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

// Autocorrection, autofill, image overlays and frame loads are left out: Booey has none of them.
// clang-format off
#define KRYS_AX_NOTIFICATIONS(macro) \
  macro(AbbreviationChanged) \
  macro(AccessKeyChanged) \
  macro(ActiveDescendantChanged) \
  macro(AnnouncementRequested) \
  macro(ARIAColumnIndexChanged) \
  macro(ARIAColumnIndexTextChanged) \
  macro(ARIANotify) \
  macro(ARIARoleDescriptionChanged) \
  macro(ARIARowIndexChanged) \
  macro(ARIARowIndexTextChanged) \
  macro(BrailleLabelChanged) \
  macro(BrailleRoleDescriptionChanged) \
  macro(CellSlotsChanged) \
  macro(CheckedStateChanged) \
  macro(ChildrenChanged) \
  macro(ColumnCountChanged) \
  macro(ColumnIndexChanged) \
  macro(ColumnSpanChanged) \
  macro(CommandChanged) \
  macro(CommandForChanged) \
  macro(ContentEditableAttributeChanged) \
  macro(ControlledObjectsChanged) \
  macro(CurrentStateChanged) \
  macro(CursorTypeChanged) \
  macro(DatetimeChanged) \
  macro(DescribedByChanged) \
  macro(DisabledStateChanged) \
  macro(DraggableStateChanged) \
  macro(DropEffectChanged) \
  macro(ExtendedDescriptionChanged) \
  macro(FlowToChanged) \
  macro(FocusableStateChanged) \
  macro(FocusedUIElementChanged) \
  macro(FontChanged) \
  macro(GrabbedStateChanged) \
  macro(HasPopupChanged) \
  macro(HiddenStateChanged) \
  macro(IdAttributeChanged) \
  macro(InertOrVisibilityChanged) \
  macro(InputTypeChanged) \
  macro(IsAtomicChanged) \
  macro(IsEditableWebAreaChanged) \
  macro(KeyShortcutsChanged) \
  macro(LabelChanged) \
  macro(LanguageChanged) \
  macro(LayoutComplete) \
  macro(LevelChanged) \
  macro(LoadComplete) \
  macro(NameChanged) \
  macro(NewDocumentLoadComplete) \
  macro(PageScrolled) \
  macro(PlaceholderChanged) \
  macro(PointerEventsChanged) \
  macro(PopoverTargetChanged) \
  macro(PositionInSetChanged) \
  macro(RoleChanged) \
  macro(RowIndexChanged) \
  macro(RowSpanChanged) \
  macro(CellScopeChanged) \
  macro(SelectedChildrenChanged) \
  macro(SelectedCellsChanged) \
  macro(SelectedStateChanged) \
  macro(SelectedTextChanged) \
  macro(SetSizeChanged) \
  macro(StyleChanged) \
  macro(TextColorChanged) \
  macro(TextCompositionBegan) \
  macro(TextCompositionEnded) \
  macro(URLChanged) \
  macro(ValueChanged) \
  macro(VisibilityChanged) \
  macro(VisitedStateChanged) \
  macro(ScrolledToAnchor) \
  macro(LiveRegionCreated) \
  macro(LiveRegionChanged) \
  macro(LiveRegionRelevantChanged) \
  macro(LiveRegionStatusChanged) \
  macro(LiveRegionAnnouncement) \
  macro(MaximumValueChanged) \
  macro(MenuListItemSelected) \
  macro(MenuListValueChanged) \
  macro(MenuClosed) \
  macro(MenuOpened) \
  macro(MinimumValueChanged) \
  macro(MultiSelectableStateChanged) \
  macro(OrientationChanged) \
  macro(RowCountChanged) \
  macro(RowCollapsed) \
  macro(RowExpanded) \
  macro(ExpandedChanged) \
  macro(InvalidStatusChanged) \
  macro(PressDidSucceed) \
  macro(PressDidFail) \
  macro(PressedStateChanged) \
  macro(RadioGroupMembershipChanged) \
  macro(ReadOnlyStatusChanged) \
  macro(RequiredStatusChanged) \
  macro(SortDirectionChanged) \
  macro(SpeakAsChanged) \
  macro(TextChanged) \
  macro(TextCompositionChanged) \
  macro(TextUnderElementChanged) \
  macro(TextSecurityChanged) \
  macro(ElementBusyChanged) \
  macro(DraggingStarted) \
  macro(DraggingEnded) \
  macro(DraggingEnteredDropZone) \
  macro(DraggingDropped) \
  macro(DraggingExitedDropZone)
// clang-format on

namespace krys::boo::ax
{
  /// @brief A change to an accessibility object that assistive technology is told about.
  /// @brief Whether text was inserted or deleted.
  enum class AXTextChange : uint8
  {
    Inserted,
    Deleted,
  };

  enum class AXNotification : uint8
  {
#define KRYS_AX_DEFINE_NOTIFICATION(name) name,
    KRYS_AX_NOTIFICATIONS(KRYS_AX_DEFINE_NOTIFICATION)
#undef KRYS_AX_DEFINE_NOTIFICATION
  };

  /// @brief @p notification's enumerator name, as logs show it.
  KRYS_NODISCARD utf8_stringview NotificationToString(AXNotification notification) noexcept;
}
