#pragma once

#include "Krystal.UI/Layout/Event/Event.hpp"
#include "Krystal.UI/Layout/Node/Node.hpp"
#include "Krystal.UI/Styles/Helpers/FlexDirection.hpp"

namespace Krys::UI
{
  void CalculateLayout(Node *node, float ownerWidth, float ownerHeight, Direction ownerDirection);

  bool CalculateLayoutInternal(Node *node, float availableWidth, float availableHeight,
                               Direction ownerDirection, SizingMode widthSizingMode,
                               SizingMode heightSizingMode, float ownerWidth, float ownerHeight,
                               bool performLayout, LayoutPassReason reason, LayoutData &layoutMarkerData,
                               uint32 depth, uint16 generationCount);
}