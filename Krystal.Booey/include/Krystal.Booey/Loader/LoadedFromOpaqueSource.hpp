#pragma once

namespace krys::boo
{
  /// @brief Whether a resource was loaded from an opaque source (e.g. a cross-origin stylesheet), which
  /// limits what may be exposed from it.
  enum class LoadedFromOpaqueSource : bool
  {
    No,
    Yes
  };
}
