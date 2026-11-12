/*
 * Copyright (C) 2005-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Accessibility/AXNotifications.hpp"
#include "Krystal.Booey/Editing/EditAction.hpp"
#include "Krystal.Booey/Editing/Selection/VisibleSelection.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class CompositeEditCommand;

  /// @brief The `inputType` of the input events for an edit of @p action; empty for actions that have none.
  KRYS_NODISCARD utf8_stringview InputTypeNameForEditingAction(EditAction action) noexcept;
  KRYS_NODISCARD bool IsInputMethodComposingForEditingAction(EditAction action) noexcept;

  enum class AllowPasswordEcho : bool
  {
    No,
    Yes
  };

  /// @brief An edit: a change to the document that's applied once, then undone and redone through the
  /// composition its top-level command makes.
  class EditCommand : public RefCounted<EditCommand>
  {
    KRYS_NON_COPY_MOVABLE(EditCommand);

  private:
    const Ref<dom::Document> _document;
    VisibleSelection _startingSelection;
    VisibleSelection _endingSelection;
    WeakPtr<CompositeEditCommand> _parent;
    EditAction _editingAction {EditAction::Unspecified};

  public:
    virtual ~EditCommand() noexcept;

    void SetParent(CompositeEditCommand *parent) noexcept;

    KRYS_NODISCARD virtual EditAction EditingAction() const noexcept;

    KRYS_NODISCARD const VisibleSelection &StartingSelection() const noexcept
    {
      return _startingSelection;
    }
    KRYS_NODISCARD const VisibleSelection &EndingSelection() const noexcept
    {
      return _endingSelection;
    }

    KRYS_NODISCARD virtual bool IsInsertTextCommand() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsSimpleEditCommand() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsCompositeEditCommand() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool IsTopLevelCommand() const noexcept
    {
      return !_parent;
    }

    virtual void DoApply() noexcept = 0;

  protected:
    explicit EditCommand(dom::Document &document,
                         EditAction editingAction = EditAction::Unspecified) noexcept;
    EditCommand(dom::Document &document, const VisibleSelection &startingSelection,
                const VisibleSelection &endingSelection) noexcept;

    KRYS_NODISCARD dom::Document &GetDocument() const noexcept
    {
      return *_document;
    }

    /// @brief Tells accessibility @p text was inserted or deleted at @p position (the selection's start if
    /// it isn't given).
    void PostTextStateChangeNotification(ax::AXTextChange change, utf8_stringview text) noexcept;
    void PostTextStateChangeNotification(ax::AXTextChange change, utf8_stringview text,
                                         const VisiblePosition &position) noexcept;
    KRYS_NODISCARD CompositeEditCommand *Parent() const noexcept
    {
      return _parent.get();
    }
    void SetStartingSelection(const VisibleSelection &selection) noexcept;
    void SetEndingSelection(const VisibleSelection &selection) noexcept;

    KRYS_NODISCARD bool IsEditingTextAreaOrTextInput() const noexcept;
  };

  enum ShouldAssumeContentIsAlwaysEditable : bool
  {
    AssumeContentIsAlwaysEditable,
    DoNotAssumeContentIsAlwaysEditable,
  };

  /// @brief An edit that undoes itself: one change to the DOM.
  class SimpleEditCommand : public EditCommand
  {
  public:
    virtual void DoUnapply() noexcept = 0;
    virtual void DoReapply() noexcept; // calls DoApply()

  protected:
    explicit SimpleEditCommand(dom::Document &document,
                               EditAction editingAction = EditAction::Unspecified) noexcept;

  private:
    KRYS_NODISCARD bool IsSimpleEditCommand() const noexcept override
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::SimpleEditCommand)
  KRYS_NODISCARD static bool IsType(const krys::boo::EditCommand &command) noexcept
  {
    return command.IsSimpleEditCommand();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
