/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/StyleContentAlignmentData.hpp"

namespace krys::boo
{
  bool StyleContentAlignmentData::IsStartward(Maybe<TextDirection> leftRightAxisDirection,
                                              bool isFlexReverse) const noexcept
  {
    switch (static_cast<ContentPosition>(_position))
    {
      case ContentPosition::Normal:
        switch (static_cast<ContentDistribution>(_distribution))
        {
          case ContentDistribution::Default:
          case ContentDistribution::Stretch:
          case ContentDistribution::SpaceBetween: return !isFlexReverse;
          default:                                return false;
        }
      case ContentPosition::Start:
      case ContentPosition::Baseline:     return true;
      case ContentPosition::End:
      case ContentPosition::LastBaseline:
      case ContentPosition::Center:       return false;
      case ContentPosition::FlexStart:    return !isFlexReverse;
      case ContentPosition::FlexEnd:      return isFlexReverse;
      case ContentPosition::Left:
        if (leftRightAxisDirection)
        {
          return leftRightAxisDirection == TextDirection::LTR;
        }
        return true;
      case ContentPosition::Right:
        if (leftRightAxisDirection)
        {
          return leftRightAxisDirection == TextDirection::RTL;
        }
        return true;
      default: krys_debug_assert("Invalid ContentPosition"); return true;
    }
  }
  bool StyleContentAlignmentData::IsEndward(Maybe<TextDirection> leftRightAxisDirection,
                                            bool isFlexReverse) const noexcept
  {
    switch (static_cast<ContentPosition>(_position))
    {
      case ContentPosition::Normal:
        switch (static_cast<ContentDistribution>(_distribution))
        {
          case ContentDistribution::Default:
          case ContentDistribution::Stretch:
          case ContentDistribution::SpaceBetween: return isFlexReverse;
          default:                                return false;
        }
      case ContentPosition::Start:
      case ContentPosition::Baseline:
      case ContentPosition::Center:       return false;
      case ContentPosition::End:
      case ContentPosition::LastBaseline: return true;
      case ContentPosition::FlexStart:    return isFlexReverse;
      case ContentPosition::FlexEnd:      return !isFlexReverse;
      case ContentPosition::Left:
        if (leftRightAxisDirection)
        {
          return leftRightAxisDirection == TextDirection::RTL;
        }
        return false;
      case ContentPosition::Right:
        if (leftRightAxisDirection)
        {
          return leftRightAxisDirection == TextDirection::LTR;
        }
        return false;
      default: krys_debug_assert("Invalid ContentPosition"); return false;
    }
  }

  bool StyleContentAlignmentData::IsCentered() const noexcept
  {
    return static_cast<ContentPosition>(_position) == ContentPosition::Center
           || static_cast<ContentDistribution>(_distribution) == ContentDistribution::SpaceAround
           || static_cast<ContentDistribution>(_distribution) == ContentDistribution::SpaceEvenly;
  }
}
