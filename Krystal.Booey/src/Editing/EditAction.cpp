/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Editing/EditAction.hpp"

namespace krys::boo
{
  utf8_stringview UndoRedoLabel(EditAction editAction) noexcept
  {
    switch (editAction)
    {
      case EditAction::Unspecified:
      case EditAction::Insert:
      case EditAction::InsertReplacement:
      case EditAction::InsertFromDrop:                 return {};
      case EditAction::SetColor:                       return u8"Set Color";
      case EditAction::SetBackgroundColor:             return u8"Set Background Color";
      case EditAction::TurnOffKerning:                 return u8"Turn Off Kerning";
      case EditAction::TightenKerning:                 return u8"Tighten Kerning";
      case EditAction::LoosenKerning:                  return u8"Loosen Kerning";
      case EditAction::UseStandardKerning:             return u8"Use Standard Kerning";
      case EditAction::TurnOffLigatures:               return u8"Turn Off Ligatures";
      case EditAction::UseStandardLigatures:           return u8"Use Standard Ligatures";
      case EditAction::UseAllLigatures:                return u8"Use All Ligatures";
      case EditAction::RaiseBaseline:                  return u8"Raise Baseline";
      case EditAction::LowerBaseline:                  return u8"Lower Baseline";
      case EditAction::SetTraditionalCharacterShape:   return u8"Set Traditional Character Shape";
      case EditAction::SetFont:                        return u8"Set Font";
      case EditAction::ChangeAttributes:               return u8"Change Attributes";
      case EditAction::AlignLeft:                      return u8"Align Left";
      case EditAction::AlignRight:                     return u8"Align Right";
      case EditAction::Center:                         return u8"Center";
      case EditAction::Justify:                        return u8"Justify";
      case EditAction::SetInlineWritingDirection:
      case EditAction::SetBlockWritingDirection:       return u8"Set Writing Direction";
      case EditAction::Subscript:                      return u8"Subscript";
      case EditAction::Superscript:                    return u8"Superscript";
      case EditAction::Underline:                      return u8"Underline";
      case EditAction::StrikeThrough:                  return u8"StrikeThrough";
      case EditAction::Outline:                        return u8"Outline";
      case EditAction::Unscript:                       return u8"Unscript";
      case EditAction::DeleteByDrag:                   return u8"Drag";
      case EditAction::Cut:                            return u8"Cut";
      case EditAction::Bold:                           return u8"Bold";
      case EditAction::Italics:                        return u8"Italics";
      case EditAction::Delete:                         return u8"Delete";
      case EditAction::Dictation:                      return u8"Dictation";
      case EditAction::Paste:                          return u8"Paste";
      case EditAction::PasteFont:                      return u8"Paste Font";
      case EditAction::PasteRuler:                     return u8"Paste Ruler";
      case EditAction::TypingDeleteSelection:
      case EditAction::TypingDeleteBackward:
      case EditAction::TypingDeleteForward:
      case EditAction::TypingDeleteWordBackward:
      case EditAction::TypingDeleteWordForward:
      case EditAction::TypingDeleteLineBackward:
      case EditAction::TypingDeleteLineForward:
      case EditAction::TypingDeletePendingComposition:
      case EditAction::TypingDeleteFinalComposition:
      case EditAction::TypingInsertText:
      case EditAction::TypingInsertLineBreak:
      case EditAction::TypingInsertParagraph:
      case EditAction::TypingInsertPendingComposition:
      case EditAction::TypingInsertFinalComposition:   return u8"Typing";
      case EditAction::CreateLink:                     return u8"Create Link";
      case EditAction::Unlink:                         return u8"Unlink";
      case EditAction::InsertUnorderedList:
      case EditAction::InsertOrderedList:              return u8"Insert List";
      case EditAction::FormatBlock:                    return u8"Formatting";
      case EditAction::Indent:                         return u8"Indent";
      case EditAction::Outdent:                        return u8"Outdent";
      case EditAction::ConvertToOrderedList:           return u8"Convert to Ordered List";
      case EditAction::ConvertToUnorderedList:         return u8"Convert to Unordered List";
      case EditAction::RemoveBackground:               return u8"Remove Background";
    }
    return {};
  }
}
