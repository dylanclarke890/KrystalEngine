/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief What shows a text field's datalist suggestions.
  enum class DataListSuggestionActivationType : uint8
  {
    /// @brief The focused field was clicked.
    ControlClicked,
    /// @brief The field's list button was clicked.
    IndicatorClicked,
    /// @brief The user changed the field's text.
    TextChanged,
    /// @brief The field's datalist or its suggestions changed.
    DataListMayHaveChanged,
  };

  /// @brief One of a text field's datalist suggestions: the value it gives the field, and its label if that
  /// differs.
  struct DataListSuggestion
  {
    utf8_string Value;
    utf8_string Label;
  };

  /// @brief The list the host shows a text field's datalist suggestions in, below the field. The field keeps
  /// the keyboard: it passes on the keys that move through the list and choose from it.
  class DataListSuggestionPicker : public RefCounted<DataListSuggestionPicker>
  {
  public:
    virtual ~DataListSuggestionPicker() noexcept = default;

    /// @brief The field's going away: the picker no longer tells it anything.
    virtual void Detach() noexcept
    {
    }

    /// @brief Hides the list; it tells its client it closed.
    virtual void Close() noexcept
    {
    }

    /// @brief A key the field passes on, by its key identifier: "Up", "Down", "Enter" or "U+001B" (Escape).
    virtual void HandleKeydownWithIdentifier(utf8_stringview identifier) noexcept
    {
      (void)identifier;
    }

    /// @brief Shows the client's current suggestions, or hides the list if there are none.
    virtual void DisplayWithActivationType(DataListSuggestionActivationType type) noexcept
    {
      (void)type;
    }
  };
}
