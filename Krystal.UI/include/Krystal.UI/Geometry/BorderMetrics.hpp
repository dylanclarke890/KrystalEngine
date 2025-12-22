#pragma once

#include "Krystal.UI/Geometry/Common.hpp"

namespace Krys::UI
{
  /// @brief The background-border metrics specify an inner and an outer rectangular area, whose corners can
  /// be rounded.
  struct BorderMetrics
  {
    /// @brief Outer corner positions (e.g. at border edge).
    CornerPositions OuterPositions;

    /// @brief Inner corner positions (e.g. at padding edge).
    CornerPositions InnerPositions;

    /// @brief Curved borders are drawn as circles (outer border) and ellipses (inner border) around the
    /// centers.
    CornerPositions CircleCenterPositions;

    /// @brief Radii of the outer edges, always circles.
    CornerSizes OuterRadii;

    /// @brief Radii of the inner edges, 2-dimensional as these can be elliptic.
    /// The inner radii is effectively the (signed) distance from the circle center to the inner edge.
    /// They can also be zero or negative, in which case a sharp corner should be drawn instead of an arc.
    CornerSizes2 InnerRadii;
  };
}