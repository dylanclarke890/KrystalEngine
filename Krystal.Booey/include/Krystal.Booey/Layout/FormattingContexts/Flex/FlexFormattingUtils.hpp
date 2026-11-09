/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class ElementBox;
  class FlexFormattingContext;
  class LogicalFlexItem;

  /// @brief Helpers for flex layout: axis orientation, gaps and the used sizes of flex items.
  class FlexFormattingUtils
  {
  private:
    const FlexFormattingContext &_flexFormattingContext;

  public:
    explicit FlexFormattingUtils(const FlexFormattingContext &flexFormattingContext) noexcept;

    KRYS_NODISCARD static bool IsMainAxisParallelWithInlineAxis(const ElementBox &flexContainer) noexcept;
    KRYS_NODISCARD static bool IsMainReversedToContentDirection(const ElementBox &flexContainer) noexcept;
    KRYS_NODISCARD static bool AreFlexLinesReversedInCrossAxis(const ElementBox &flexContainer) noexcept;

    // FIXME: These values should probably be computed by FlexFormattingContext and get passed in to
    // FlexLayout.
    KRYS_NODISCARD static LayoutUnit MainAxisGapValue(const ElementBox &flexContainer,
                                                      LayoutUnit flexContainerContentBoxWidth) noexcept;
    KRYS_NODISCARD static LayoutUnit CrossAxisGapValue(const ElementBox &flexContainer,
                                                       LayoutUnit flexContainerContentBoxHeight) noexcept;

    KRYS_NODISCARD static ContentPosition
      LogicalJustifyContentPosition(const ElementBox &flexContainer,
                                    ContentPosition justifyContentPosition) noexcept;

    KRYS_NODISCARD LayoutUnit UsedMinimumSizeInMainAxis(const LogicalFlexItem &flexItem) const noexcept;
    KRYS_NODISCARD Maybe<LayoutUnit>
      UsedMaximumSizeInMainAxis(const LogicalFlexItem &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit UsedMaxContentSizeInMainAxis(const LogicalFlexItem &flexItem) const noexcept;
    KRYS_NODISCARD LayoutUnit UsedSizeInCrossAxis(const LogicalFlexItem &flexItem,
                                                  LayoutUnit maxAxisConstraint) const noexcept;

  private:
    KRYS_NODISCARD const FlexFormattingContext &FormattingContext() const noexcept
    {
      return _flexFormattingContext;
    }
  };
}
