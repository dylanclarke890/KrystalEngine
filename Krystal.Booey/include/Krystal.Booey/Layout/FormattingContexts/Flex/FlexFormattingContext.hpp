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

#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexFormattingConstraints.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexFormattingUtils.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexLayout.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Flex/FlexRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/FormattingConstraints.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationUtils.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo::layout
{
  class Box;
  class BoxGeometry;
  class ElementBox;
  class LayoutState;

  // This class implements the layout logic for flex formatting contexts.
  // https://www.w3.org/TR/css-flexbox-1/
  /// @brief Lays out the in-flow children of a flex container.
  class FlexFormattingContext
  {
    KRYS_NON_COPY_MOVABLE(FlexFormattingContext);

  private:
    const ElementBox &_flexBox;
    LayoutState &_globalLayoutState;
    const FlexFormattingUtils _flexFormattingUtils;
    const IntegrationUtils _integrationUtils;

  public:
    FlexFormattingContext(const ElementBox &flexBox, LayoutState &globalLayoutState) noexcept;

    void Layout(const ConstraintsForFlexContent &constraints) noexcept;
    KRYS_NODISCARD IntrinsicWidthConstraints ComputedIntrinsicWidthConstraints() noexcept;

    KRYS_NODISCARD const ElementBox &Root() const noexcept
    {
      return _flexBox;
    }
    KRYS_NODISCARD const FlexFormattingUtils &FormattingUtils() const noexcept
    {
      return _flexFormattingUtils;
    }

    KRYS_NODISCARD const BoxGeometry &GeometryForFlexItem(const Box &flexItem) const noexcept;
    KRYS_NODISCARD BoxGeometry &GeometryForFlexItem(const Box &flexItem) noexcept;

    KRYS_NODISCARD const IntegrationUtils &GetIntegrationUtils() const noexcept
    {
      return _integrationUtils;
    }

  private:
    KRYS_NODISCARD FlexLayout::LogicalFlexItems
      ConvertFlexItemsToLogicalSpace(const ConstraintsForFlexContent &constraints) noexcept;
    void SetFlexItemsGeometry(const FlexLayout::LogicalFlexItems &logicalFlexItemList,
                              const FlexLayout::LogicalFlexItemRects &logicalRects,
                              const ConstraintsForFlexContent &constraints) noexcept;
    void PositionOutOfFlowChildren() noexcept;
  };
}
