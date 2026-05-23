#pragma once

#include "Krystal.UI/Layout/Api/Forward.hpp"
#include "Krystal.UI/Styles/Enums/Direction.hpp"
#include "Krystal.UI/Styles/Enums/Edge.hpp"

namespace Krys::UI
{
  float NodeLayoutGetLeft(NodeConstRef node);

  float NodeLayoutGetTop(NodeConstRef node);

  float NodeLayoutGetRight(NodeConstRef node);

  float NodeLayoutGetBottom(NodeConstRef node);

  float NodeLayoutGetWidth(NodeConstRef node);

  float NodeLayoutGetHeight(NodeConstRef node);

  Direction NodeLayoutGetDirection(NodeConstRef node);

  bool NodeLayoutGetHadOverflow(NodeConstRef node);

  /// @brief Get the computed values for these nodes after performing layout. If they were set using point
  /// values then the returned value will be the same as NodeStyleGetXXX. However if they were set using a
  /// percentage value then the returned value is the computed value used during layout.
  float NodeLayoutGetMargin(NodeConstRef node, Edge edge);

  float NodeLayoutGetBorder(NodeConstRef node, Edge edge);

  float NodeLayoutGetPadding(NodeConstRef node, Edge edge);

  /// @brief Return the measured height of the node, before layout rounding
  float NodeLayoutGetRawHeight(NodeConstRef node);

  /// Return the measured width of the node, before layout rounding
  float NodeLayoutGetRawWidth(NodeConstRef node);
}