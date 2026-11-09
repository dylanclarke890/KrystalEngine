/*
 * Copyright (c) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class ElementBox;
  class InlineLayoutState;
  class LayoutState;
}

namespace krys::boo::layout_integration
{
  /// @brief Lays out @p box with its own formatting context (through its renderer), optionally under width
  /// and height constraints, and copies the result to its box geometry.
  void LayoutWithFormattingContextForBox(const layout::ElementBox &box, Maybe<LayoutUnit> widthConstraint,
                                         Maybe<LayoutUnit> heightConstraint,
                                         layout::LayoutState &layoutState) noexcept;

  enum class LogicalWidthType : uint8
  {
    PreferredMaximum,
    PreferredMinimum,
    MaxContent,
    MinContent
  };
  KRYS_NODISCARD LayoutUnit FormattingContextRootLogicalWidthForType(
    const layout::ElementBox &box, LogicalWidthType logicalWidthType) noexcept;

  enum class LogicalHeightType : uint8
  {
    MinContent
  };
  KRYS_NODISCARD LayoutUnit FormattingContextRootLogicalHeightForType(
    const layout::ElementBox &box, LogicalHeightType logicalHeightType) noexcept;

  /// @brief Lays out a block in inline content (through the block container's renderer) at the line's logical
  /// top left, and copies the result to its box geometry and the inline layout state.
  void LayoutWithFormattingContextForBlockInInline(const layout::ElementBox &block,
                                                   LayoutPoint blockLineLogicalTopLeft,
                                                   layout::InlineLayoutState &inlineLayoutState,
                                                   layout::LayoutState &layoutState) noexcept;
}
