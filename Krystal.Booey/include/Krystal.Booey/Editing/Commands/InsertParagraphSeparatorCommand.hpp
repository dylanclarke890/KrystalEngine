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
  /// @brief Replaces the selection with a paragraph break, splitting the block (and the inline elements) the
  /// caret is in.
  class InsertParagraphSeparatorCommand final : public CompositeEditCommand
  {
  private:
    RefPtr<EditingStyle> _style;
    bool _mustUseDefaultParagraphElement;

  public:
    InsertParagraphSeparatorCommand(dom::Document &document, bool mustUseDefaultParagraphElement,
                                    EditAction editingAction) noexcept;
    ~InsertParagraphSeparatorCommand() noexcept override;

    KRYS_NODISCARD static Ref<InsertParagraphSeparatorCommand>
      Create(dom::Document &document, bool useDefaultParagraphElement = false,
             EditAction editingAction = EditAction::Insert) noexcept;

  private:
    void DoApply() noexcept override;

    void CalculateStyleBeforeInsertion(const dom::Position &position) noexcept;
    void ApplyStyleAfterInsertion(dom::Node *originalEnclosingBlock) noexcept;
    void GetAncestorsInsideBlock(const dom::Node *insertionNode, dom::Element *outerBlock,
                                 List<RefPtr<dom::Element>> &ancestors) noexcept;
    Ref<dom::Element> CloneHierarchyUnderNewBlock(const List<RefPtr<dom::Element>> &ancestors,
                                                  Ref<dom::Element> &&blockToInsert) noexcept;
    KRYS_NODISCARD bool ShouldUseDefaultParagraphElement(dom::Node *enclosingBlock) const noexcept;
    KRYS_NODISCARD bool PreservesTypingStyle() const noexcept override;
  };
}
