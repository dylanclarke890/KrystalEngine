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
