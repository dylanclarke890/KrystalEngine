/*
 * Copyright (C) 2011 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/RenderOverflow.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief What a box split across fragments has in one of them: its logical left and width there, and its
  /// overflow in it.
  class RenderBoxFragmentInfo
  {
    KRYS_NON_COPYABLE(RenderBoxFragmentInfo);

  private:
    LayoutUnit _logicalLeft;
    LayoutUnit _logicalWidth;
    bool _isShifted {false};
    UniquePtr<RenderOverflow> _overflow;

  public:
    RenderBoxFragmentInfo(LayoutUnit logicalLeft, LayoutUnit logicalWidth, bool isShifted) noexcept
        : _logicalLeft(logicalLeft), _logicalWidth(logicalWidth), _isShifted(isShifted)
    {
    }

    KRYS_NODISCARD LayoutUnit LogicalLeft() const noexcept
    {
      return _logicalLeft;
    }
    KRYS_NODISCARD LayoutUnit LogicalWidth() const noexcept
    {
      return _logicalWidth;
    }

    void ShiftLogicalLeft(LayoutUnit delta) noexcept
    {
      _logicalLeft += delta;
      _isShifted = true;
    }

    KRYS_NODISCARD bool IsShifted() const noexcept
    {
      return _isShifted;
    }

    void CreateOverflow(const LayoutRect &layoutOverflow, const LayoutRect &visualOverflow) noexcept
    {
      _overflow = CreateUnique<RenderOverflow>(layoutOverflow, visualOverflow);
    }
    KRYS_NODISCARD RenderOverflow *Overflow() const noexcept
    {
      return _overflow.get();
    }
    void ClearOverflow() noexcept
    {
      _overflow = nullptr;
    }
  };
}
