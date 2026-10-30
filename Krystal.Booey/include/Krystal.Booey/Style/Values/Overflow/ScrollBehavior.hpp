#pragma once

namespace krys::boo::style
{
  // <'scroll-behavior'> = auto | smooth
  // https://drafts.csswg.org/css-overflow-3/#smooth-scrolling
  enum class ScrollBehavior : bool
  {
    Auto,
    Smooth
  };
}
