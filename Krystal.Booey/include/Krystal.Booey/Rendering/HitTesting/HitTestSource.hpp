#pragma once

namespace krys::boo
{
  /// @brief Whether a hit test was triggered by the user or by script.
  enum class HitTestSource : bool
  {
    Script,
    User
  };
}
