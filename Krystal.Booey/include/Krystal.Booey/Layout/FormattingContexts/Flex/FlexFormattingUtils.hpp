#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
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
