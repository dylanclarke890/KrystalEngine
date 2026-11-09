/*
 * Copyright (C) 2004-2018 Apple Inc. All rights reserved.
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
