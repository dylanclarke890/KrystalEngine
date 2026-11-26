#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  /// @brief Options passed through CSS serialisation.
  /// @note Upstream's URL replacement fields (used only when saving web archives) aren't supported, so this
  /// is currently empty; it's kept so serialisation has a place for options later.
  struct SerialisationContext
  {
  };

  KRYS_NODISCARD const SerialisationContext &DefaultSerialisationContext() noexcept;
}
