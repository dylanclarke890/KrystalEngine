#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::layout_integration
{
  // FIXME: layoutWithFormattingContextForBox, formattingContextRootLogicalWidthForType,
  // formattingContextRootLogicalHeightForType and layoutWithFormattingContextForBlockInInline, which the
  // render view's layout state calls, once there's LineLayout.

  enum class LogicalWidthType : uint8
  {
    PreferredMaximum,
    PreferredMinimum,
    MaxContent,
    MinContent
  };

  enum class LogicalHeightType : uint8
  {
    MinContent
  };
}
