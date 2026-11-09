/*
 * Copyright (C) 2006-2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Editing/Selection/VisibleSelection.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  /// @brief How a temporary selection is made.
  enum class TemporarySelectionOption : uint8
  {
    RevealSelection,
    DoNotSetFocus,
    /// @brief Don't propagate selection changes to the client layer.
    IgnoreSelectionChanges,
    /// @brief Force the render tree to update selection state. Only respected on iOS.
    EnableAppearanceUpdates,
    SmoothScroll,
    DelegateMainFrameScroll,
    RevealSelectionBounds,
    UserTriggered,
    ForceCenterScroll,
    OnlyAllowForwardScrolling
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::TemporarySelectionOption, 10uz);

namespace krys::boo
{
  /// @brief Selects something for as long as it lives (to reveal it, say), then restores the selection.
  class TemporarySelectionChange
  {
    KRYS_NON_COPY_MOVABLE(TemporarySelectionChange);

  private:
    enum class IsTemporarySelection : bool
    {
      No,
      Yes
    };

    RefPtr<dom::Document> _document;
    EnumSet<TemporarySelectionOption> _options;
    Maybe<VisibleSelection> _selectionToRestore;
    bool _wasIgnoringSelectionChanges {false};

  public:
    explicit TemporarySelectionChange(dom::Document &document,
                                      Maybe<VisibleSelection> temporarySelection = std::nullopt,
                                      EnumSet<TemporarySelectionOption> options = {}) noexcept;
    ~TemporarySelectionChange() noexcept;

    /// @brief Keeps the temporary selection: nothing is restored.
    void Invalidate() noexcept;

  private:
    void SetSelection(const VisibleSelection &selection, IsTemporarySelection isTemporarySelection) noexcept;
  };
}
