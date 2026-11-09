/*
 * Copyright (C) 2005, 2006, 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Editing/Commands/CompositeEditCommand.hpp"

namespace krys::boo
{
  /// @brief Adds document markers to the text an insertion made.
  class TextInsertionMarkerSupplier : public RefCounted<TextInsertionMarkerSupplier>
  {
  public:
    virtual ~TextInsertionMarkerSupplier() noexcept = default;
    virtual void AddMarkersToTextNode(dom::Text &text, uint32 offsetOfInsertion,
                                      utf8_stringview textInserted) noexcept = 0;

  protected:
    TextInsertionMarkerSupplier() noexcept = default;
  };

  /// @brief Replaces the selection with a line of text (or a tab, which goes in a tab span).
  // FIXME: Smart lists, with InsertListCommand (on Cocoa platforms only).
  class InsertTextCommand : public CompositeEditCommand
  {
    friend class TypingCommand;

  public:
    enum RebalanceType : bool
    {
      RebalanceLeadingAndTrailingWhitespaces,
      RebalanceAllWhitespaces
    };

  private:
    utf8_string _text;
    AllowPasswordEcho _allowPasswordEcho {AllowPasswordEcho::Yes};
    bool _selectInsertedText;
    RebalanceType _rebalanceType;
    RefPtr<TextInsertionMarkerSupplier> _markerSupplier;

  public:
    InsertTextCommand(dom::Document &document, utf8_stringview text,
                      Ref<TextInsertionMarkerSupplier> &&markerSupplier, EditAction editingAction) noexcept;
    InsertTextCommand(dom::Document &document, utf8_stringview text, AllowPasswordEcho allowPasswordEcho,
                      bool selectInsertedText, RebalanceType rebalanceType,
                      EditAction editingAction) noexcept;

    KRYS_NODISCARD static Ref<InsertTextCommand>
      Create(dom::Document &document, utf8_stringview text, AllowPasswordEcho allowPasswordEcho,
             bool selectInsertedText = false,
             RebalanceType rebalanceType = RebalanceLeadingAndTrailingWhitespaces,
             EditAction editingAction = EditAction::Insert) noexcept;
    KRYS_NODISCARD static Ref<InsertTextCommand>
      CreateWithMarkerSupplier(dom::Document &document, utf8_stringview text,
                               Ref<TextInsertionMarkerSupplier> &&markerSupplier,
                               EditAction editingAction = EditAction::Insert) noexcept;

  private:
    void DoApply() noexcept override;
    KRYS_NODISCARD bool IsInsertTextCommand() const noexcept override
    {
      return true;
    }

    dom::Position PositionInsideTextNode(const dom::Position &position) noexcept;
    dom::Position InsertTab(const dom::Position &position) noexcept;

    bool PerformTrivialReplace(utf8_stringview text, bool selectInsertedText) noexcept;
    bool PerformOverwrite(utf8_stringview text, bool selectInsertedText) noexcept;
    void SetEndingSelectionWithoutValidation(const dom::Position &startPosition,
                                             const dom::Position &endPosition) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::InsertTextCommand)
  KRYS_NODISCARD static bool IsType(const krys::boo::EditCommand &command) noexcept
  {
    return command.IsInsertTextCommand();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
