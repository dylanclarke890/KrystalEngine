#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief What an edit does, which names its undo step and the input type of its input events.
  enum class EditAction : uint8
  {
    AlignLeft,
    AlignRight,
    Bold,
    Center,
    ChangeAttributes,
    ConvertToOrderedList,
    ConvertToUnorderedList,
    CreateLink,
    Cut,
    Delete,
    DeleteByDrag,
    Dictation,
    FormatBlock,
    Indent,
    Insert,
    InsertFromDrop,
    InsertOrderedList,
    InsertReplacement,
    InsertUnorderedList,
    Italics,
    Justify,
    LoosenKerning,
    LowerBaseline,
    Outdent,
    Outline,
    Paste,
    PasteFont,
    PasteRuler,
    RaiseBaseline,
    RemoveBackground,
    SetBackgroundColor,
    SetBlockWritingDirection,
    SetColor,
    SetFont,
    SetInlineWritingDirection,
    SetTraditionalCharacterShape,
    StrikeThrough,
    Subscript,
    Superscript,
    TightenKerning,
    TurnOffKerning,
    TurnOffLigatures,
    TypingDeleteBackward,
    TypingDeleteFinalComposition,
    TypingDeleteForward,
    TypingDeleteLineBackward,
    TypingDeleteLineForward,
    TypingDeletePendingComposition,
    TypingDeleteSelection,
    TypingDeleteWordBackward,
    TypingDeleteWordForward,
    TypingInsertFinalComposition,
    TypingInsertLineBreak,
    TypingInsertParagraph,
    TypingInsertPendingComposition,
    TypingInsertText,
    Underline,
    Unlink,
    Unscript,
    Unspecified,
    UseAllLigatures,
    UseStandardKerning,
    UseStandardLigatures,
  };

  /// @brief The name of an undo or redo step for @p editAction, such as "Typing" or "Paste"; empty for
  /// actions that have none.
  // FIXME: Localised names, once the host can give them.
  KRYS_NODISCARD utf8_stringview UndoRedoLabel(EditAction editAction) noexcept;
}
