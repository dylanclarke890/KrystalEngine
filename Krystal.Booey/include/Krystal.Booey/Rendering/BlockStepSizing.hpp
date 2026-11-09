/**
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

#pragma once

#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class RenderBox;
  class RenderStyle;
  class WritingMode;
}

/// @brief Block step sizing (`block-step-size`): rounding a block-level box's outer size up to a multiple of
/// the step.
namespace krys::boo::BlockStepSizing
{
  /// @brief Whether the child's block step properties are the ones block layout supports.
  KRYS_NODISCARD bool ChildHasSupportedStyle(const RenderStyle &childStyle) noexcept;

  /// @brief The space that rounds @p boxOuterSize up to a multiple of @p stepSize.
  KRYS_NODISCARD LayoutUnit ComputeExtraSpace(LayoutUnit stepSize, LayoutUnit boxOuterSize) noexcept;

  void DistributeExtraSpaceToChildMargins(RenderBox &child, LayoutUnit extraSpace,
                                          WritingMode containingBlockWritingMode) noexcept;
  void DistributeExtraSpaceToChildPadding(RenderBox &child, LayoutUnit extraSpace,
                                          WritingMode containingBlockWritingMode) noexcept;
  void DistributeExtraSpaceToChildContentArea(RenderBox &child, LayoutUnit extraSpace,
                                              WritingMode containingBlockWritingMode) noexcept;
}
