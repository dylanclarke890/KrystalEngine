#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::web
{
  /// @brief The platform whose editing conventions a document follows (`Settings::EditingBehaviorType`).
  enum class EditingBehaviorType : uint8
  {
    Mac,
    Windows,
    Unix,
    iOS,
  };

  /// @brief The editing conventions of a platform: one query for each place the platforms differ in how
  /// carets move, selections extend and text is deleted.
  class EditingBehavior
  {
  private:
    EditingBehaviorType _type;

  public:
    constexpr explicit EditingBehavior(EditingBehaviorType type) noexcept : _type(type)
    {
    }

    /// @brief Whether a point above the first line or below the last one of an editable area puts the caret
    /// at the start or end of the content, rather than at the same horizontal position on that line
    /// (Windows).
    KRYS_NODISCARD constexpr bool ShouldMoveCaretToHorizontalBoundaryWhenPastTopOrBottom() const noexcept
    {
      return _type != EditingBehaviorType::Windows;
    }

    /// @brief Whether every selection has a direction, however it was made, rather than only those extended
    /// from the keyboard (Mac and iOS).
    KRYS_NODISCARD constexpr bool ShouldConsiderSelectionAsDirectional() const noexcept
    {
      return _type != EditingBehaviorType::Mac && _type != EditingBehaviorType::iOS;
    }

    /// @brief Whether revealing the selection scrolls it to the centre, rather than just into view.
    KRYS_NODISCARD constexpr bool ShouldCenterAlignWhenSelectionIsRevealed() const noexcept
    {
      return _type == EditingBehaviorType::Mac || _type == EditingBehaviorType::iOS;
    }

    /// @brief Whether extending a range selection to a boundary (a line's or paragraph's end, say) grows it
    /// at the end it's extended towards, rather than moving its extent.
    KRYS_NODISCARD constexpr bool ShouldAlwaysGrowSelectionWhenExtendingToBoundary() const noexcept
    {
      return _type == EditingBehaviorType::Mac || _type == EditingBehaviorType::iOS;
    }

    /// @brief Whether a context menu click selects what it's on.
    KRYS_NODISCARD constexpr bool ShouldSelectOnContextualMenuClick() const noexcept
    {
      return _type == EditingBehaviorType::Mac;
    }

    /// @brief Whether extending a selection by word, line or paragraph can carry it straight across its base,
    /// rather than stopping at a caret there first (Mac and iOS).
    KRYS_NODISCARD constexpr bool ShouldExtendSelectionByWordOrLineAcrossCaret() const noexcept
    {
      return _type != EditingBehaviorType::Mac && _type != EditingBehaviorType::iOS;
    }

    /// @brief Whether moving right by a word goes to the start of the next word, past the spaces, rather than
    /// to the end of the current one. (Moving left always goes to the start of a word.)
    KRYS_NODISCARD constexpr bool ShouldSkipSpaceWhenMovingRight() const noexcept
    {
      return _type == EditingBehaviorType::Windows;
    }

    /// @brief Whether double-clicking a word selects the whitespace after it too.
    KRYS_NODISCARD constexpr bool ShouldSelectTrailingWhitespaceOnDoubleClick() const noexcept
    {
      return _type == EditingBehaviorType::Windows;
    }

    /// @brief Whether moving from a range selection's start or end starts at its extent either way, rather
    /// than at the start or end asked for.
    KRYS_NODISCARD constexpr bool ShouldAlwaysExtendSelectionFromExtentEndpoint() const noexcept
    {
      return _type != EditingBehaviorType::Mac && _type != EditingBehaviorType::iOS;
    }

    /// @brief Whether focusing a text field puts the caret at its end, rather than selecting all of it.
    KRYS_NODISCARD constexpr bool ShouldMoveSelectionToEndWhenFocusingTextInput() const noexcept
    {
      return _type == EditingBehaviorType::iOS;
    }

    /// @brief Whether smart delete (with the host's smart insert and delete on) applies to every deletion,
    /// not only to a word selection.
    KRYS_NODISCARD constexpr bool ShouldAlwaysSmartDelete() const noexcept
    {
      return _type == EditingBehaviorType::iOS;
    }

    /// @brief Whether smart insert and delete also adjust the blank paragraphs around pasted or deleted
    /// paragraphs.
    KRYS_NODISCARD constexpr bool ShouldSmartInsertDeleteParagraphs() const noexcept
    {
      return _type == EditingBehaviorType::iOS;
    }
  };
}
