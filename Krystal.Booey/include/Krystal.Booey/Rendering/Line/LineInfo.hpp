/*
 * Copyright (C) 2013 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief What the legacy line layout knows of the line it's building.
  class LineInfo
  {
  private:
    bool _isFirstLine {true};
    bool _isLastLine {false};
    bool _isEmpty {true};
    uint32 _runsFromLeadingWhitespace {0};

  public:
    LineInfo() noexcept = default;

    KRYS_NODISCARD bool IsFirstLine() const noexcept
    {
      return _isFirstLine;
    }
    KRYS_NODISCARD bool IsLastLine() const noexcept
    {
      return _isLastLine;
    }
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _isEmpty;
    }
    KRYS_NODISCARD uint32 RunsFromLeadingWhitespace() const noexcept
    {
      return _runsFromLeadingWhitespace;
    }
    void ResetRunsFromLeadingWhitespace() noexcept
    {
      _runsFromLeadingWhitespace = 0;
    }
    void IncrementRunsFromLeadingWhitespace() noexcept
    {
      _runsFromLeadingWhitespace++;
    }

    void SetFirstLine(bool firstLine) noexcept
    {
      _isFirstLine = firstLine;
    }
    void SetLastLine(bool lastLine) noexcept
    {
      _isLastLine = lastLine;
    }
    void SetEmpty(bool empty) noexcept
    {
      _isEmpty = empty;
    }
  };
}
