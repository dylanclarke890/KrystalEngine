#pragma once

namespace krys::boo::style
{
  // When the span refers to a logical axis that needs to be resolved to physical
  // axis, this determines whether to use the writing mode of the element's
  // containing block or the element itself.
  enum class PositionAreaSelf : bool
  {
    // Use the writing mode of the element's containing block.
    No,

    // Use the writing mode of the element itself.
    Yes
  };
}
