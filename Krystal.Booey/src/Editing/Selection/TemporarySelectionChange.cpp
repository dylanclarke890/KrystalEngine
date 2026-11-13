/*
 * Copyright (C) 2006-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Editing/Selection/TemporarySelectionChange.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include <utility>

namespace krys::boo
{
  TemporarySelectionChange::TemporarySelectionChange(dom::Document &document,
                                                     Maybe<VisibleSelection> temporarySelection,
                                                     EnumSet<TemporarySelectionOption> options) noexcept
      : _document(ShareRefPtr(&document)), _options(options),
        _wasIgnoringSelectionChanges(document.GetEditor().IgnoreSelectionChanges())
  {
    if (options.Contains(TemporarySelectionOption::IgnoreSelectionChanges))
    {
      document.GetEditor().SetIgnoreSelectionChanges(true);
    }

    if (temporarySelection)
    {
      _selectionToRestore = document.Selection().Selection();
      SetSelection(*temporarySelection, IsTemporarySelection::Yes);
    }
  }

  void TemporarySelectionChange::Invalidate() noexcept
  {
    if (auto document = std::exchange(_document, nullptr))
    {
      document->GetEditor().SetIgnoreSelectionChanges(false, Editor::RevealSelection::No);
    }
  }

  TemporarySelectionChange::~TemporarySelectionChange() noexcept
  {
    if (!_document)
    {
      return;
    }

    if (_selectionToRestore)
    {
      SetSelection(*_selectionToRestore, IsTemporarySelection::No);
    }

    if (_options.Contains(TemporarySelectionOption::IgnoreSelectionChanges))
    {
      auto revealSelection = _options.Contains(TemporarySelectionOption::RevealSelection)
                               ? Editor::RevealSelection::Yes
                               : Editor::RevealSelection::No;
      _document->GetEditor().SetIgnoreSelectionChanges(_wasIgnoringSelectionChanges, revealSelection);
    }
  }

  void TemporarySelectionChange::SetSelection(const VisibleSelection &selection,
                                              IsTemporarySelection isTemporarySelection) noexcept
  {
    using SetSelectionOption = FrameSelection::SetSelectionOption;
    auto options = FrameSelection::DefaultSetSelectionOptions();

    if (_options.Contains(TemporarySelectionOption::UserTriggered))
    {
      options.Add(SetSelectionOption::IsUserTriggered);
    }

    if (_options.Contains(TemporarySelectionOption::DoNotSetFocus))
    {
      options.Add(SetSelectionOption::DoNotSetFocus);
    }

    if (isTemporarySelection == IsTemporarySelection::Yes)
    {
      if (_options.Contains(TemporarySelectionOption::RevealSelection))
      {
        options.Add(SetSelectionOption::RevealSelection);
      }
      if (_options.Contains(TemporarySelectionOption::DelegateMainFrameScroll))
      {
        options.Add(SetSelectionOption::DelegateMainFrameScroll);
      }
      if (_options.Contains(TemporarySelectionOption::SmoothScroll))
      {
        options.Add(SetSelectionOption::SmoothScroll);
      }
      if (_options.Contains(TemporarySelectionOption::RevealSelectionBounds))
      {
        options.Add(SetSelectionOption::RevealSelectionBounds);
      }
      if (_options.Contains(TemporarySelectionOption::ForceCenterScroll))
      {
        options.Add(SetSelectionOption::ForceCenterScroll);
      }
      if (_options.Contains(TemporarySelectionOption::OnlyAllowForwardScrolling))
      {
        options.Add(SetSelectionOption::OnlyAllowForwardScrolling);
      }
    }

    _document->Selection().SetSelection(selection, options);

    // The selection is brought up to date and revealed at the next rendering update, by which time a
    // temporary one is gone, so it's revealed now.
    if (isTemporarySelection == IsTemporarySelection::Yes
        && _options.Contains(TemporarySelectionOption::RevealSelection))
    {
      _document->Selection().UpdateAndRevealSelection(
        _options.Contains(TemporarySelectionOption::SmoothScroll) ? ScrollBehavior::Smooth
                                                                  : ScrollBehavior::Instant,
        _options.Contains(TemporarySelectionOption::RevealSelectionBounds)
          ? RevealExtentOption::DoNotRevealExtent
          : RevealExtentOption::RevealExtent,
        _options.Contains(TemporarySelectionOption::ForceCenterScroll) ? ForceCenterScroll::Yes
                                                                       : ForceCenterScroll::No,
        _options.Contains(TemporarySelectionOption::OnlyAllowForwardScrolling)
          ? OnlyAllowForwardScrolling::Yes
          : OnlyAllowForwardScrolling::No);
    }
  }
}
