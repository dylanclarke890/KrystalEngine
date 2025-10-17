#pragma once

#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI.Layout/Enums/MeasureMode.hpp"
#include <limits>

namespace Krys::UI::Layout
{
  struct Size
  {
    float Width;
    float Height;
  };

  using NodeRef = class Node *;
  using NodeConstRef = const class Node *;

  /// @brief Returns the computed dimensions of the node, following the constraints of `widthMode` and
  /// `heightMode`:
  /// - Undefined: The parent has not imposed any constraint on the child. It can be whatever size it wants.
  /// - AtMost: The child can be as large as it wants up to the specified size.
  /// - Exactly: The parent has determined an exact size for the child. The child is going to be given those
  /// bounds regardless of how big it wants to be.
  /// @returns the size of the leaf node, measured under the given constraints.
  typedef Size (*MeasureFunc)(NodeConstRef node, float width, MeasureMode widthMode, float height,
                              MeasureMode heightMode);

  /// @returns a defined offset to baseline (ascent).
  typedef float (*BaselineFunc)(NodeConstRef node, float width, float height);

  /// Called when a node is marked dirty. (i.e. a property of the node has changed)
  typedef void (*DirtiedFunc)(NodeConstRef node);

  typedef NodeRef (*CloneNodeFunc)(NodeConstRef oldNode, NodeConstRef owner, size_t childIndex);

  typedef struct Config *ConfigRef;
  typedef const struct Config *ConfigConstRef;

  constexpr auto Undefined = std::numeric_limits<float>::quiet_NaN();
}