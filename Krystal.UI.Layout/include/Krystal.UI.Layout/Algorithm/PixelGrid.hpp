#pragma once

#include "Krystal.UI.Layout/Node/Node.hpp"

namespace Krys::UI::Layout
{
  /// @brief Round a point value to the nearest physical pixel based on DPI (pointScaleFactor)
  float RoundValueToPixelGrid(double value, double pointScaleFactor, bool forceCeil, bool forceFloor);

  /// @brief Round the layout results of a node and its subtree to the pixel grid.
  void RoundLayoutResultsToPixelGrid(Node *node, double absoluteLeft, double absoluteTop);
}