#pragma once

#include "Krystal.Lib/ComparisonHelpers.hpp"
#include "Krystal.Lib/NullableFloat.hpp"
#include "Krystal.UI.Layout/Algorithm/FlexDirection.hpp"
#include "Krystal.UI.Layout/Enums/Dimension.hpp"
#include "Krystal.UI.Layout/Enums/Direction.hpp"
#include "Krystal.UI.Layout/Node/Node.hpp"

namespace Krys::UI::Layout
{
  inline float PaddingAndBorderForAxis(const Node *const node, const FlexDirection axis,
                                       const Direction direction, const float widthSize)
  {
    return node->GetStyle().ComputeInlineStartPaddingAndBorder(axis, direction, widthSize)
           + node->GetStyle().ComputeInlineEndPaddingAndBorder(axis, direction, widthSize);
  }

  inline NullableFloat BoundAxisWithinMinAndMax(const Node *const node, const Direction direction,
                                                const FlexDirection axis, const NullableFloat value,
                                                const float axisSize, const float widthSize)
  {
    NullableFloat min;
    NullableFloat max;

    if (IsColumn(axis))
    {
      min = node->GetStyle().ResolvedMinDimension(direction, Dimension::Height, axisSize, widthSize);
      max = node->GetStyle().ResolvedMaxDimension(direction, Dimension::Height, axisSize, widthSize);
    }
    else if (IsRow(axis))
    {
      min = node->GetStyle().ResolvedMinDimension(direction, Dimension::Width, axisSize, widthSize);
      max = node->GetStyle().ResolvedMaxDimension(direction, Dimension::Width, axisSize, widthSize);
    }

    if (max >= NullableFloat {0} && value > max)
    {
      return max;
    }

    if (min >= NullableFloat {0} && value < min)
    {
      return min;
    }

    return value;
  }

  /// @brief Like boundAxisWithinMinAndMax but also ensures that the value doesn't go below the padding and
  /// border amount.
  inline float BoundAxis(const Node *const node, const FlexDirection axis, const Direction direction,
                         const float value, const float axisSize, const float widthSize)
  {
    return MaxOrDefined(
      BoundAxisWithinMinAndMax(node, direction, axis, NullableFloat {value}, axisSize, widthSize).Value(),
      PaddingAndBorderForAxis(node, axis, direction, widthSize));
  }
}