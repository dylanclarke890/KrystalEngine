#pragma once

namespace krys::boo
{
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-getanimationsoptions
  struct GetAnimationsOptions
  {
    /// @brief Whether to include the animations of the element's descendants.
    bool Subtree {false};
  };
}
