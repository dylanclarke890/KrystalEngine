/*
 * Copyright (C) 2017-2020 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo
{
  class RenderBlockFlow;
}

namespace krys::boo::layout
{
  class PlacedFloats;
}

namespace krys::boo::layout_integration
{
  class InlineContent;

  /// @brief How far a line moves down to fit in its column.
  struct LineAdjustment
  {
    LayoutUnit Offset {0_lu};
    bool IsFirstAfterPageBreak {false};
  };

  /// @brief The line layout restarts from to move lines to the next column for widows.
  struct LayoutRestartLine
  {
    uint64 Index {0};
    LayoutUnit Offset;
  };

  /// @brief How far each line of @p inlineContent moves down to fit in the columns of @p flow, keeping the
  /// widows and orphans together, and the line to lay out from again when that needs another pass.
  KRYS_NODISCARD Pair<List<LineAdjustment>, Maybe<LayoutRestartLine>>
    ComputeAdjustmentsForPagination(const InlineContent &inlineContent,
                                    const layout::PlacedFloats &placedFloats, bool allowLayoutRestart,
                                    RenderBlockFlow &flow) noexcept;
  /// @brief Moves the lines and boxes of @p inlineContent by @p adjustments.
  void AdjustLinePositionsForPagination(InlineContent &inlineContent,
                                        const List<LineAdjustment> &adjustments) noexcept;
}
