/*
 * Copyright (C) 2015 Igalia S.L. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"

namespace krys::boo
{
  class StyleContentAlignmentData
  {
  public:
    constexpr StyleContentAlignmentData() noexcept = default;

    // Style data for Content-Distribution properties: align-content, justify-content.
    // <content-distribution> || [ <overflow-position>? && <content-position> ]
    constexpr StyleContentAlignmentData(ContentPosition position,
                                        ContentDistribution distribution = ContentDistribution::Default,
                                        OverflowAlignment overflow = OverflowAlignment::Default) noexcept
        : _position(ToUnderlying(position)), _distribution(ToUnderlying(distribution)),
          _overflow(ToUnderlying(overflow))
    {
    }

    void SetPosition(ContentPosition position) noexcept
    {
      _position = ToUnderlying(position);
    }
    void SetDistribution(ContentDistribution distribution) noexcept
    {
      _distribution = ToUnderlying(distribution);
    }
    void SetOverflow(OverflowAlignment overflow) noexcept
    {
      _overflow = ToUnderlying(overflow);
    }

    KRYS_NODISCARD ContentPosition Position() const noexcept
    {
      return static_cast<ContentPosition>(_position);
    }
    KRYS_NODISCARD ContentDistribution Distribution() const noexcept
    {
      return static_cast<ContentDistribution>(_distribution);
    }
    KRYS_NODISCARD OverflowAlignment Overflow() const noexcept
    {
      return static_cast<OverflowAlignment>(_overflow);
    }
    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return ContentPosition::Normal == static_cast<ContentPosition>(_position)
             && ContentDistribution::Default == static_cast<ContentDistribution>(_distribution);
    }
    KRYS_NODISCARD bool IsStartward(Maybe<TextDirection> leftRightAxisDirection = null,
                                    bool isFlexReverse = false) const noexcept;
    KRYS_NODISCARD bool IsEndward(Maybe<TextDirection> leftRightAxisDirection = null,
                                  bool isFlexReverse = false) const noexcept;
    // leftRightAxisDirection is only needed for justify-content (invalid for align-content).
    // Pass null if neither the inline axis nor the physical left-right axis matches the justify-content axis
    // (e.g. in flexbox).
    KRYS_NODISCARD bool IsCentered() const noexcept;

    KRYS_NODISCARD bool operator==(const StyleContentAlignmentData &) const noexcept = default;

  private:
    uint16 _position : 4 {0};     // ContentPosition
    uint16 _distribution : 3 {0}; // ContentDistribution
    uint16 _overflow : 2 {0};     // OverflowAlignment
  };
}
