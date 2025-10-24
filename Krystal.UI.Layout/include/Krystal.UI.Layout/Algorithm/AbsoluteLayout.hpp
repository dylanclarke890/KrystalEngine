#pragma once

#include "Krystal.UI.Layout/Event/Event.hpp"
#include "Krystal.UI.Layout/Node/Node.hpp"

namespace Krys::UI::Layout
{
  void LayoutAbsoluteChild(const Node *containingNode, const Node *node, Node *child,
                           float containingBlockWidth, float containingBlockHeight,
                           Styles::SizingMode widthMode, Styles::Direction direction,
                           LayoutData &layoutMarkerData, uint32 depth, uint32 generationCount);

  /// @brief Returns true if some absolute descendant has new layout.
  bool LayoutAbsoluteDescendants(Node *containingNode, Node *currentNode, Styles::SizingMode widthSizingMode,
                                 Styles::Direction currentNodeDirection, LayoutData &layoutMarkerData,
                                 uint32 currentDepth, uint32 generationCount,
                                 float currentNodeMainOffsetFromContainingBlock,
                                 float currentNodeCrossOffsetFromContainingBlock,
                                 float containingNodeAvailableInnerWidth,
                                 float containingNodeAvailableInnerHeight);
}