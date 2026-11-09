/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  class KeyboardEvent;
}

namespace krys::boo::html
{
  /// @brief The options a TypeAhead searches.
  class TypeAheadDataSource
  {
  public:
    virtual ~TypeAheadDataSource() noexcept = default;

    KRYS_NODISCARD virtual int32 IndexOfSelectedOption() const noexcept = 0;
    KRYS_NODISCARD virtual int32 OptionCount() const noexcept = 0;
    KRYS_NODISCARD virtual dom::DOMString OptionAtIndex(int32 index) const noexcept = 0;
  };

  /// @brief Finds the option the user is typing the start of.
  class TypeAhead
  {
  public:
    enum class MatchMode : uint8
    {
      MatchPrefix,
      CycleFirstChar,
      MatchIndex,
    };

  private:
    TypeAheadDataSource *_dataSource {nullptr};
    MonotonicTime::duration _lastTypeTime {};
    char32 _repeatingChar {0};
    utf32_string _buffer;

  public:
    explicit TypeAhead(TypeAheadDataSource *dataSource) noexcept;

    /// @return The index of the matching option, or -1 if none matches.
    KRYS_NODISCARD int32 HandleEvent(dom::KeyboardEvent &event, EnumSet<MatchMode> matchMode) noexcept;

    /// @brief Forgets what was typed, so the next key starts a new search.
    void ResetSession() noexcept
    {
      _lastTypeTime = {};
      _repeatingChar = 0;
      _buffer.clear();
    }
  };
}
