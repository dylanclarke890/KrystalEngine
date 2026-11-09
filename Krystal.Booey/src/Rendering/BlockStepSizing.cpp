/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/BlockStepSizing.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::BlockStepSizing
{
  bool ChildHasSupportedStyle(const RenderStyle &childStyle) noexcept
  {
    return childStyle.GetBlockStepInsert() == BlockStepInsert::MarginBox
           && childStyle.GetBlockStepAlign() == BlockStepAlign::Auto
           && childStyle.GetBlockStepRound() == BlockStepRound::Up;
  }

  LayoutUnit ComputeExtraSpace(LayoutUnit stepSize, LayoutUnit boxOuterSize) noexcept
  {
    if (!stepSize)
    {
      return {};
    }

    if (!boxOuterSize)
    {
      return stepSize;
    }

    if (auto remainder = IntMod(boxOuterSize, stepSize))
    {
      return stepSize - remainder;
    }
    return {};
  }

  void DistributeExtraSpaceToChildMargins(RenderBox &child, LayoutUnit extraSpace,
                                          WritingMode containingBlockWritingMode) noexcept
  {
    auto halfExtraSpace = extraSpace / 2;
    child.SetMarginBefore(child.MarginBefore(containingBlockWritingMode) + halfExtraSpace);
    child.SetMarginAfter(child.MarginAfter(containingBlockWritingMode) + halfExtraSpace);
  }

  void DistributeExtraSpaceToChildPadding(RenderBox &, LayoutUnit, WritingMode) noexcept
  {
    // FIXME: Not implemented yet.
    krys_debug_assert(false);
  }

  void DistributeExtraSpaceToChildContentArea(RenderBox &, LayoutUnit, WritingMode) noexcept
  {
    // FIXME: Not implemented yet.
    krys_debug_assert(false);
  }
}
