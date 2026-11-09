/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/LayoutSize.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/FormattingConstraints.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::layout
{
  /// @brief The space inline content is laid out in, with the formatting root's visual left and border box
  /// size.
  class ConstraintsForInlineContent : public ConstraintsForInFlowContent
  {
  private:
    LayoutUnit _visualLeft;
    LayoutSize _formattingRootBorderBoxSize;

  public:
    ConstraintsForInlineContent(const ConstraintsForInFlowContent &genericContraints, LayoutUnit visualLeft,
                                LayoutSize formattingRootBorderBoxSize) noexcept
        : ConstraintsForInFlowContent(genericContraints.Horizontal(), genericContraints.LogicalTop(),
                                      InlineContent),
          _visualLeft(visualLeft), _formattingRootBorderBoxSize(formattingRootBorderBoxSize)
    {
    }

    KRYS_NODISCARD LayoutUnit VisualLeft() const noexcept
    {
      return _visualLeft;
    }
    KRYS_NODISCARD LayoutSize FormattingRootBorderBoxSize() const noexcept
    {
      return _formattingRootBorderBoxSize;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::layout::ConstraintsForInlineContent)
  KRYS_NODISCARD static bool IsType(
    const krys::boo::layout::ConstraintsForInFlowContent &constraints) noexcept
  {
    return constraints.IsConstraintsForInlineContent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
