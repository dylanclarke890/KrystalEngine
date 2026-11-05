#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  /// @brief Whether moving a position may cross from editable content into content that isn't (or back).
  enum class EditingBoundaryCrossingRule : uint8
  {
    CanCrossEditingBoundary,
    CannotCrossEditingBoundary,
    CanSkipOverEditingBoundary
  };

  /// @brief What makes a node count as editable.
  /// FIXME: `HasEditableAXRole`, once there's accessibility.
  enum class EditableType : uint8
  {
    ContentIsEditable
  };
}
