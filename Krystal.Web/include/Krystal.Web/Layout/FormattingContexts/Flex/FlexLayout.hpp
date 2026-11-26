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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Layout/FormattingContexts/Flex/FlexFormattingConstraints.hpp"
#include "Krystal.Web/Layout/FormattingContexts/Flex/FlexRect.hpp"
#include "Krystal.Web/Layout/FormattingContexts/Flex/LogicalFlexItem.hpp"
#include "Krystal.Web/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyleConstants.hpp"

namespace krys::web
{
  class RenderStyle;
}

namespace krys::web::layout
{
  struct FlexBaseAndHypotheticalMainSize;
  class FlexFormattingContext;
  class FlexFormattingUtils;
  struct PositionAndMargins;

  // This class implements the layout logic for flex formatting contexts.
  // https://www.w3.org/TR/css-flexbox-1/
  /// @brief Runs the flex layout algorithm over logical flex items and returns their logical rects.
  class FlexLayout
  {
  public:
    using LogicalFlexItems = List<LogicalFlexItem>;
    using LogicalFlexItemRects = List<FlexRect>;

  private:
    using FlexBaseAndHypotheticalMainSizeList = List<FlexBaseAndHypotheticalMainSize>;
    using LineRanges = List<IndexRange>;
    using SizeList = List<LayoutUnit>;
    using PositionAndMarginsList = List<PositionAndMargins>;
    using LinesCrossSizeList = List<LayoutUnit>;
    using LinesCrossPositionList = List<LayoutUnit>;

    FlexFormattingContext &_flexFormattingContext;

  public:
    explicit FlexLayout(FlexFormattingContext &flexFormattingContext) noexcept;

    KRYS_NODISCARD LogicalFlexItemRects Layout(const ConstraintsForFlexContent &flexContainerConstraints,
                                               const LogicalFlexItems &flexItems) noexcept;

  private:
    KRYS_NODISCARD FlexBaseAndHypotheticalMainSizeList FlexBaseAndHypotheticalMainSizeForFlexItems(
      const LogicalFlexItems &flexItems, bool isSizedUnderMinMaxConstraints) const noexcept;

    KRYS_NODISCARD static LayoutUnit
      FlexContainerInnerMainSize(const ConstraintsForFlexContent::AxisGeometry &mainAxisGeometry) noexcept;

    KRYS_NODISCARD LineRanges ComputeFlexLines(
      const LogicalFlexItems &flexItems, LayoutUnit flexContainerInnerMainSize,
      const FlexBaseAndHypotheticalMainSizeList &flexBaseAndHypotheticalMainSizeList) const noexcept;

    KRYS_NODISCARD SizeList ComputeMainSizeForFlexItems(
      const LogicalFlexItems &flexItems, const LineRanges &lineRanges, LayoutUnit flexContainerInnerMainSize,
      const FlexBaseAndHypotheticalMainSizeList &flexBaseAndHypotheticalMainSizeList) const noexcept;

    KRYS_NODISCARD SizeList HypotheticalCrossSizeForFlexItems(const LogicalFlexItems &flexItems,
                                                              const SizeList &flexItemsMainSizeList) noexcept;

    KRYS_NODISCARD LinesCrossSizeList CrossSizeForFlexLines(
      const LineRanges &lineRanges, const ConstraintsForFlexContent::AxisGeometry &crossAxis,
      const LogicalFlexItems &flexItems, const SizeList &flexItemsHypotheticalCrossSizeList) const noexcept;

    void StretchFlexLines(LinesCrossSizeList &flexLinesCrossSizeList, size_t numberOfLines,
                          Maybe<LayoutUnit> crossAxisAvailableSpace) const noexcept;

    KRYS_NODISCARD static bool CollapseNonVisibleFlexItems() noexcept;

    KRYS_NODISCARD SizeList
      ComputeCrossSizeForFlexItems(const LogicalFlexItems &flexItems, const LineRanges &lineRanges,
                                   const LinesCrossSizeList &flexLinesCrossSizeList,
                                   const SizeList &flexItemsHypotheticalCrossSizeList) const noexcept;

    KRYS_NODISCARD PositionAndMarginsList HandleMainAxisAlignment(
      LayoutUnit availableMainSpace, const LineRanges &lineRanges, const LogicalFlexItems &flexItems,
      const SizeList &flexItemsMainSizeList) const noexcept;

    KRYS_NODISCARD PositionAndMarginsList HandleCrossAxisAlignmentForFlexItems(
      const LogicalFlexItems &flexItems, const LineRanges &lineRanges, const SizeList &flexItemsCrossSizeList,
      const LinesCrossSizeList &flexLinesCrossSizeList) const noexcept;

    KRYS_NODISCARD LinesCrossPositionList HandleCrossAxisAlignmentForFlexLines(
      Maybe<LayoutUnit> crossAxisAvailableSpace, const LineRanges &lineRanges,
      LinesCrossSizeList &flexLinesCrossSizeList) const noexcept;

    KRYS_NODISCARD LayoutUnit MainAxisAvailableSpaceForItemAlignment(LayoutUnit mainAxisAvailableSpace,
                                                                     size_t numberOfFlexItems) const noexcept;

    KRYS_NODISCARD LayoutUnit CrossAxisAvailableSpaceForLineSizingAndAlignment(
      LayoutUnit crossAxisAvailableSpace, size_t numberOfFlexLines) const noexcept;

    KRYS_NODISCARD bool IsSingleLineFlexContainer() const noexcept;

    KRYS_NODISCARD const ElementBox &FlexContainer() const noexcept;

    KRYS_NODISCARD const RenderStyle &FlexContainerStyle() const noexcept;

    KRYS_NODISCARD const FlexFormattingContext &FormattingContext() const noexcept;

    KRYS_NODISCARD const FlexFormattingUtils &FormattingUtils() const noexcept;
  };
}
