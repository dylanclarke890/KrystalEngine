/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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
  class LocalFrame;

  /// @brief The base of commands that insert text, which can apply themselves to a selection other than the
  /// frame's.
  class TextInsertionBaseCommand : public CompositeEditCommand
  {
  public:
    ~TextInsertionBaseCommand() noexcept override = default;

  protected:
    explicit TextInsertionBaseCommand(dom::Document &document,
                                      EditAction editingAction = EditAction::Unspecified) noexcept;

    static void ApplyTextInsertionCommand(LocalFrame *frame, TextInsertionBaseCommand &command,
                                          const VisibleSelection &selectionForInsertion,
                                          const VisibleSelection &endingSelection) noexcept;
  };

  KRYS_NODISCARD utf8_string DispatchBeforeTextInsertedEvent(utf8_stringview text,
                                                             const VisibleSelection &selectionForInsertion,
                                                             bool insertionIsForUpdatingComposition) noexcept;
  KRYS_NODISCARD bool CanAppendNewLineFeedToSelection(const VisibleSelection &selection) noexcept;

  // LineOperation should define member function "opeartor (size_t lineOffset, size_t lineLength, bool
  // isLastLine)". lienLength doesn't include the newline character. So the value of lineLength could be 0.
  template <typename TLineOperation>
  void ForEachLineInString(utf8_stringview string, const TLineOperation &operation) noexcept
  {
    size_t offset = 0;
    size_t newline;
    while ((newline = string.find(u8'\n', offset)) != utf8_stringview::npos)
    {
      operation(offset, newline - offset, false);
      offset = newline + 1;
    }
    if (!offset)
    {
      operation(0, string.size(), true);
    }
    else
    {
      size_t length = string.size();
      if (length != offset)
      {
        operation(offset, length - offset, true);
      }
    }
  }
}
