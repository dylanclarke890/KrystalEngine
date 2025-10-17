#pragma once

#include "Krystal.UI.Layout/Node/Node.hpp"

namespace Krys::UI::Layout
{
  /// @brief Calculate baseline represented as an offset from the top edge of the node.
  float CalculateBaseline(const Node *node);

  /// @brief Whether any of the children of this node participate in baseline alignment
  bool IsBaselineLayout(const Node *node);
}