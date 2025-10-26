#pragma once

#include "Krystal.UI/Layout/Event/Event.hpp"
#include "Krystal.UI/Layout/Node/Node.hpp"
#include "Krystal.UI/Styles/Helpers/FlexDirection.hpp"

namespace Krys::UI
{
  /// @brief Given an offset to an edge, returns the offset to the opposite edge on the same axis. This
  /// assumes that the width/height of both nodes is determined at this point.
  inline float GetPositionOfOppositeEdge(float position, FlexDirection axis,
                                         const Node *const containingNode, const Node *const node)
  {
    return containingNode->GetLayout().GetMeasuredDimension(ToDimension(axis))
           - node->GetLayout().GetMeasuredDimension(ToDimension(axis)) - position;
  }

  inline void SetChildTrailingPosition(const Node *const node, Node *const child,
                                       const FlexDirection axis)
  {
    child->SetLayoutPosition(
      GetPositionOfOppositeEdge(child->GetLayout().GetPosition(FlexStartEdge(axis)), axis, node,
                                child),
      FlexEndEdge(axis));
  }

  inline bool NeedsTrailingPosition(const FlexDirection axis)
  {
    return axis == FlexDirection::RowReverse || axis == FlexDirection::ColumnReverse;
  }
}