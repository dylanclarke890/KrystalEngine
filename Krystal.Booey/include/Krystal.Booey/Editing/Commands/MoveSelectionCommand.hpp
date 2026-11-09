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

#include "Krystal.Booey/DOM/Ranges/Position.hpp"
#include "Krystal.Booey/Editing/Commands/CompositeEditCommand.hpp"

namespace krys::boo::dom
{
  class DocumentFragment;
}

namespace krys::boo
{
  /// @brief Moves the selection, dropped as @p fragment, to a position: deletes the selection, then inserts
  /// the fragment there.
  class MoveSelectionCommand final : public CompositeEditCommand
  {
  private:
    const Ref<dom::DocumentFragment> _fragment;
    dom::Position _position;
    bool _smartInsert;
    bool _smartDelete;

  public:
    MoveSelectionCommand(Ref<dom::DocumentFragment> &&fragment, const dom::Position &position,
                         bool smartInsert, bool smartDelete) noexcept;

    KRYS_NODISCARD static Ref<MoveSelectionCommand> Create(Ref<dom::DocumentFragment> &&fragment,
                                                           const dom::Position &position,
                                                           bool smartInsert = false,
                                                           bool smartDelete = false) noexcept;

  private:
    void DoApply() noexcept override;
    KRYS_NODISCARD EditAction EditingAction() const noexcept override;
    KRYS_NODISCARD bool ShouldDispatchInputEvents() const noexcept final
    {
      return false;
    }
  };
}
