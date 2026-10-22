#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief The platform whose editing conventions a document follows.
  // There are multiple editing details that are different on Windows than Macintosh.
  // We use a single switch for all of them. Some examples:
  //
  //    1) Clicking below the last line of an editable area puts the caret at the end
  //       of the last line on Mac, but in the middle of the last line on Windows.
  //    2) Pushing the down arrow key on the last line puts the caret at the end of the
  //       last line on Mac, but does nothing on Windows. A similar case exists on the
  //       top line.
  enum class EditingBehaviorType : uint8
  {
    Mac,
    Windows,
    Unix,
    iOS,
  };

  /// @brief The editing conventions of a platform, one query for each place they differ.
  class EditingBehavior
  {
  private:
    EditingBehaviorType _type;

  public:
    constexpr explicit EditingBehavior(EditingBehaviorType type) noexcept : _type(type)
    {
    }

    // When extending a selection beyond the top or bottom boundary of an editable area,
    // maintain the horizontal position on Windows but extend it to the boundary of the editable
    // content on Mac.
    KRYS_NODISCARD constexpr bool ShouldMoveCaretToHorizontalBoundaryWhenPastTopOrBottom() const noexcept
    {
      return _type != EditingBehaviorType::Windows;
    }

    // On Windows, selections should always be considered as directional, regardless if it is
    // mouse-based or keyboard-based.
    KRYS_NODISCARD constexpr bool ShouldConsiderSelectionAsDirectional() const noexcept
    {
      return _type != EditingBehaviorType::Mac && _type != EditingBehaviorType::iOS;
    }

    // On Mac, when revealing a selection (for example as a result of a Find operation on the Browser),
    // content should be scrolled such that the selection gets certer aligned.
    KRYS_NODISCARD constexpr bool ShouldCenterAlignWhenSelectionIsRevealed() const noexcept
    {
      return _type == EditingBehaviorType::Mac || _type == EditingBehaviorType::iOS;
    }

    // On Mac, style is considered present when present at the beginning of selection. On other platforms,
    // style has to be present throughout the selection.
    KRYS_NODISCARD constexpr bool ShouldToggleStyleBasedOnStartOfSelection() const noexcept
    {
      return _type == EditingBehaviorType::Mac || _type == EditingBehaviorType::iOS;
    }

    // Standard Mac behavior when extending to a boundary is grow the selection rather than leaving the base
    // in place and moving the extent. Matches NSTextView.
    KRYS_NODISCARD constexpr bool ShouldAlwaysGrowSelectionWhenExtendingToBoundary() const noexcept
    {
      return _type == EditingBehaviorType::Mac || _type == EditingBehaviorType::iOS;
    }

    // On Mac, when processing a contextual click, the object being clicked upon should be selected.
    KRYS_NODISCARD constexpr bool ShouldSelectOnContextualMenuClick() const noexcept
    {
      return _type == EditingBehaviorType::Mac;
    }

    // On Linux, should be able to get and insert spelling suggestions without selecting the misspelled word.
    KRYS_NODISCARD constexpr bool ShouldAllowSpellingSuggestionsWithoutSelection() const noexcept
    {
      return _type == EditingBehaviorType::Unix;
    }

    // On Mac and Windows, pressing backspace (when it isn't handled otherwise) should navigate back.
    KRYS_NODISCARD constexpr bool ShouldNavigateBackOnBackspace() const noexcept
    {
      return _type != EditingBehaviorType::Unix;
    }

    // On Mac, selecting backwards by word/line from the middle of a word/line, and then going
    // forward leaves the caret back in the middle with no selection, instead of directly selecting
    // to the other end of the line/word (Unix/Windows behavior).
    KRYS_NODISCARD constexpr bool ShouldExtendSelectionByWordOrLineAcrossCaret() const noexcept
    {
      return _type != EditingBehaviorType::Mac && _type != EditingBehaviorType::iOS;
    }

    // Based on native behavior, when using ctrl(alt)+arrow to move caret by word, ctrl(alt)+left arrow moves
    // caret to immediately before the word in all platforms, for example, the word break positions are:
    // "|abc |def |hij |opq". But ctrl+right arrow moves caret to "abc |def |hij |opq" on Windows and
    // "abc| def| hij| opq|" on Mac and Linux.
    KRYS_NODISCARD constexpr bool ShouldSkipSpaceWhenMovingRight() const noexcept
    {
      return _type == EditingBehaviorType::Windows;
    }

    // On iOS the last entered character in a secure filed is shown momentarily, removing and adding back the
    // space when deleting password cause space been showed insecurely.
    KRYS_NODISCARD constexpr bool ShouldRebalanceWhiteSpacesInSecureField() const noexcept
    {
      return _type != EditingBehaviorType::iOS;
    }

    KRYS_NODISCARD constexpr bool ShouldSelectBasedOnDictionaryLookup() const noexcept
    {
      return _type == EditingBehaviorType::Mac || _type == EditingBehaviorType::iOS;
    }

    // Linux and Windows always extend selections from the extent endpoint.
    KRYS_NODISCARD constexpr bool ShouldAlwaysExtendSelectionFromExtentEndpoint() const noexcept
    {
      return _type != EditingBehaviorType::Mac && _type != EditingBehaviorType::iOS;
    }

    // On iOS, we don't want to select all the text when focusing a field. Instead, match platform behavior by
    // going to the end of the line.
    KRYS_NODISCARD constexpr bool ShouldMoveSelectionToEndWhenFocusingTextInput() const noexcept
    {
      return _type == EditingBehaviorType::iOS;
    }

    // On iOS, when smart delete is on, it is always on, and should do not additional checks (i.e.
    // TextGranularity::WordGranularity).
    KRYS_NODISCARD constexpr bool ShouldAlwaysSmartDelete() const noexcept
    {
      return _type == EditingBehaviorType::iOS;
    }

    // On iOS, we should turn on smart insert and delete and newlines around paragraphs to match UIKit
    // behaviour.
    KRYS_NODISCARD constexpr bool ShouldSmartInsertDeleteParagraphs() const noexcept
    {
      return _type == EditingBehaviorType::iOS;
    }
  };
}
